#include "CpuAccumulatorDriver.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <ratio>
#include <thread>
#include <utility>
#include <vector>
#include "core/CameraSystem.h"
#include "core/Color.h"
#include "core/ColorMap.h"
#include "core/CpuAccumulator.h"
#include "core/HistogramBuffer.h"
#include "core/ThreadLocalRng.h"

namespace chaoskit::ui {

namespace {

constexpr uint64_t kInitialBatchSize = 4096;
constexpr uint64_t kMinBatchSize = 4096;
constexpr uint64_t kMaxBatchSize = 262144;
constexpr double kTargetBatchMs = 10.0;

unsigned int resolveThreadCount(unsigned int requested) {
  if (requested > 0) {
    return requested;
  }
  return (std::max)(1u, std::thread::hardware_concurrency());
}

uint64_t scaleBatchSize(uint64_t currentBatchSize, double elapsedMs) {
  if (elapsedMs > 0.0) {
    const double ratio = std::clamp(kTargetBatchMs / elapsedMs, 0.5, 2.0);
    return std::clamp(
        static_cast<uint64_t>(static_cast<double>(currentBatchSize) * ratio),
        kMinBatchSize, kMaxBatchSize);
  }
  return (std::min)(currentBatchSize * 2, kMaxBatchSize);
}

QString extractExceptionMessage(const std::exception_ptr& error) {
  try {
    std::rethrow_exception(error);
  } catch (const std::exception& e) {
    return QString::fromUtf8(e.what());
  } catch (...) {
    return QStringLiteral("Unknown CPU accumulation error");
  }
}

}  // namespace

struct CpuAccumulatorDriver::WorkerSlot {
  std::unique_ptr<core::CpuAccumulator> accumulator;
  uint64_t localGen = 0;
  uint64_t batchEpoch = 0;
  uint64_t stepBudget = 0;
  std::exception_ptr error;
};

uint64_t CpuAccumulatorDriver::mergeWorkerHistograms(
    const std::vector<std::unique_ptr<WorkerSlot>>& workerSlots,
    core::HistogramBuffer& stagingBuffer) {
  bool initializedStaging = false;
  uint64_t totalPlotted = 0;
  for (const auto& slot : workerSlots) {
    if (!slot->accumulator) {
      continue;
    }
    if (!initializedStaging) {
      stagingBuffer = slot->accumulator->histogram();
      totalPlotted = slot->accumulator->plottedIterations();
      initializedStaging = true;
      continue;
    }
    const core::HistogramBuffer& srcHist = slot->accumulator->histogram();
    if (srcHist.width() == stagingBuffer.width() &&
        srcHist.height() == stagingBuffer.height()) {
      core::Color* dst = stagingBuffer.data();
      const core::Color* src = srcHist.data();
      const size_t count = stagingBuffer.size();
      for (size_t i = 0; i < count; ++i) {
        dst[i] += src[i];
      }
      totalPlotted += slot->accumulator->plottedIterations();
    }
  }
  return totalPlotted;
}

CpuAccumulatorDriver::CpuAccumulatorDriver(QObject* parent,
                                           unsigned int threadCount)
    : QObject(parent), threadCount_(resolveThreadCount(threadCount)) {
  workerSlots_.reserve(threadCount_);
  for (unsigned int i = 0; i < threadCount_; ++i) {
    workerSlots_.push_back(std::make_unique<WorkerSlot>());
  }
  workers_.reserve(threadCount_);
  for (unsigned int i = 0; i < threadCount_; ++i) {
    workers_.emplace_back(&CpuAccumulatorDriver::workerLoop, this,
                          static_cast<size_t>(i));
  }
  coordinator_ = std::thread(&CpuAccumulatorDriver::coordinatorLoop, this);
}

CpuAccumulatorDriver::~CpuAccumulatorDriver() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stopRequested_ = true;
  }
  cv_.notify_all();
  if (coordinator_.joinable()) {
    coordinator_.join();
  }

  {
    std::lock_guard<std::mutex> poolLock(poolMutex_);
    poolStopRequested_ = true;
  }
  poolWorkCv_.notify_all();
  for (std::thread& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

bool CpuAccumulatorDriver::isRunning() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return running_;
}

uint64_t CpuAccumulatorDriver::plottedIterations() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return committedPlottedIterations_;
}

bool CpuAccumulatorDriver::syncSnapshotIfChanged(
    uint64_t& lastVersion, core::HistogramBuffer& bufferOut,
    uint64_t* plottedIterationsOut) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (lastVersion == snapshotVersion_) {
    return false;
  }
  bufferOut = committedBuffer_;
  if (plottedIterationsOut != nullptr) {
    *plottedIterationsOut = committedPlottedIterations_;
  }
  lastVersion = snapshotVersion_;
  return true;
}

void CpuAccumulatorDriver::snapshot(core::HistogramBuffer& bufferOut,
                                    uint64_t* plottedIterationsOut) const {
  std::lock_guard<std::mutex> lock(mutex_);
  bufferOut = committedBuffer_;
  if (plottedIterationsOut != nullptr) {
    *plottedIterationsOut = committedPlottedIterations_;
  }
}

void CpuAccumulatorDriver::setSystem(const core::CameraSystem& system) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    system_ = system;
    ++generation_;
    ++snapshotVersion_;
    committedBuffer_.clear();
    committedPlottedIterations_ = 0;
  }
  cv_.notify_one();
}

void CpuAccumulatorDriver::clearSystem() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    system_.reset();
    ++generation_;
    ++snapshotVersion_;
    committedBuffer_.clear();
    committedPlottedIterations_ = 0;
  }
  cv_.notify_one();
}

void CpuAccumulatorDriver::setColorMap(const core::ColorMap* colorMap) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (colorMap_ == colorMap) {
      return;
    }
    colorMap_ = colorMap;
    ++generation_;
    ++snapshotVersion_;
    committedBuffer_.clear();
    committedPlottedIterations_ = 0;
  }
  cv_.notify_one();
}

void CpuAccumulatorDriver::setSize(uint32_t width, uint32_t height) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (width_ == width && height_ == height) {
      return;
    }
    width_ = width;
    height_ = height;
    ++generation_;
    ++snapshotVersion_;
    committedBuffer_.resize(width, height);
    committedPlottedIterations_ = 0;
  }
  cv_.notify_one();
}

void CpuAccumulatorDriver::setRunning(bool running) {
  bool changed = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_ != running) {
      running_ = running;
      changed = true;
    }
  }
  if (changed) {
    cv_.notify_one();
    emit runningChanged(running);
  }
}

void CpuAccumulatorDriver::clear() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    ++generation_;
    ++snapshotVersion_;
    committedBuffer_.clear();
    committedPlottedIterations_ = 0;
  }
  cv_.notify_one();
}

void CpuAccumulatorDriver::workerLoop(size_t workerIndex) {
  WorkerSlot& slot = *workerSlots_[workerIndex];
  uint64_t seenEpoch = 0;

  while (true) {
    std::optional<core::CameraSystem> systemCopy = std::nullopt;
    const core::ColorMap* colorMap = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t gen = 0;
    uint64_t budget = 0;

    {
      std::unique_lock<std::mutex> lock(poolMutex_);
      poolWorkCv_.wait(lock, [&]() {
        return poolStopRequested_ || slot.batchEpoch != seenEpoch;
      });
      if (poolStopRequested_) {
        break;
      }
      seenEpoch = slot.batchEpoch;
      systemCopy = batchSystem_;
      colorMap = batchColorMap_;
      width = batchWidth_;
      height = batchHeight_;
      gen = batchGeneration_;
      budget = slot.stepBudget;
    }

    std::exception_ptr error;
    try {
      if (slot.localGen != gen || !slot.accumulator) {
        slot.localGen = gen;
        slot.accumulator = std::make_unique<core::CpuAccumulator>(
            *systemCopy, width, height, colorMap,
            std::make_shared<core::ThreadLocalRng>());
      }
      slot.accumulator->accumulate(budget);
    } catch (...) {
      slot.accumulator.reset();
      error = std::current_exception();
    }

    {
      std::lock_guard<std::mutex> lock(poolMutex_);
      slot.error = error;
      if (--remainingWorkers_ == 0) {
        poolDoneCv_.notify_one();
      }
    }
  }
}

void CpuAccumulatorDriver::coordinatorLoop() {
  core::HistogramBuffer stagingBuffer;
  uint64_t localGen = 0;
  uint64_t batchSize = kInitialBatchSize;
  uint64_t currentEpoch = 0;

  while (true) {
    std::optional<core::CameraSystem> activeSystem = std::nullopt;
    const core::ColorMap* activeColorMap = nullptr;
    uint32_t activeWidth = 0;
    uint32_t activeHeight = 0;

    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [&]() {
        return stopRequested_ ||
               (running_ && system_.has_value() && width_ > 0 && height_ > 0);
      });
      if (stopRequested_) {
        break;
      }
      if (localGen != generation_) {
        localGen = generation_;
        batchSize = kInitialBatchSize;
      }
      activeSystem = system_;
      activeColorMap = colorMap_;
      activeWidth = width_;
      activeHeight = height_;
    }

    const auto start = std::chrono::steady_clock::now();
    {
      std::lock_guard<std::mutex> poolLock(poolMutex_);
      ++currentEpoch;
      batchSystem_ = std::move(activeSystem);
      batchColorMap_ = activeColorMap;
      batchWidth_ = activeWidth;
      batchHeight_ = activeHeight;
      batchGeneration_ = localGen;

      const uint64_t share = batchSize / threadCount_;
      const uint64_t remainder = batchSize % threadCount_;
      for (size_t i = 0; i < workerSlots_.size(); ++i) {
        workerSlots_[i]->stepBudget = share + (i < remainder ? 1u : 0u);
        workerSlots_[i]->error = nullptr;
        workerSlots_[i]->batchEpoch = currentEpoch;
      }
      remainingWorkers_ = workerSlots_.size();
    }
    poolWorkCv_.notify_all();

    {
      std::unique_lock<std::mutex> poolLock(poolMutex_);
      poolDoneCv_.wait(poolLock, [&]() { return remainingWorkers_ == 0; });
    }

    const auto end = std::chrono::steady_clock::now();
    const double elapsedMs =
        (end - start) / std::chrono::duration<double, std::milli>(1.0);

    QString errorMessage;
    for (const auto& slot : workerSlots_) {
      if (slot->error) {
        errorMessage = extractExceptionMessage(slot->error);
        break;
      }
    }

    if (!errorMessage.isEmpty()) {
      bool isCurrentGen = false;
      bool wasRunning = false;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        isCurrentGen = !stopRequested_ && (generation_ == localGen);
        if (isCurrentGen) {
          wasRunning = running_;
          running_ = false;
        }
      }
      if (isCurrentGen) {
        if (wasRunning) {
          emit runningChanged(false);
        }
        emit errorOccurred(errorMessage);
      }
      continue;
    }

    bool stale = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stale = stopRequested_ || (generation_ != localGen);
    }
    if (stale) {
      continue;
    }

    const uint64_t totalPlotted =
        mergeWorkerHistograms(workerSlots_, stagingBuffer);

    bool committed = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!stopRequested_ && generation_ == localGen) {
        std::swap(committedBuffer_, stagingBuffer);
        committedPlottedIterations_ = totalPlotted;
        ++snapshotVersion_;
        committed = true;
      }
    }

    if (committed) {
      emit batchCompleted();
    }

    batchSize = scaleBatchSize(batchSize, elapsedMs);
  }
}

}  // namespace chaoskit::ui
