#include "CpuAccumulatorDriver.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <ratio>
#include <utility>
#include "core/CameraSystem.h"
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

}  // namespace

CpuAccumulatorDriver::CpuAccumulatorDriver(QObject* parent)
    : QObject(parent), worker_(&CpuAccumulatorDriver::workerLoop, this) {}

CpuAccumulatorDriver::~CpuAccumulatorDriver() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stopRequested_ = true;
  }
  cv_.notify_all();
  if (worker_.joinable()) {
    worker_.join();
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

void CpuAccumulatorDriver::workerLoop() {
  std::unique_ptr<core::CpuAccumulator> accumulator;
  core::HistogramBuffer stagingBuffer;
  uint64_t localGen = 0;
  uint64_t batchSize = kInitialBatchSize;

  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [&]() {
        return stopRequested_ ||
               (running_ && system_.has_value() && width_ > 0 && height_ > 0);
      });
      if (stopRequested_) {
        break;
      }
      if (localGen != generation_ || !accumulator) {
        localGen = generation_;
        accumulator = std::make_unique<core::CpuAccumulator>(
            *system_, width_, height_, colorMap_,
            std::make_shared<core::ThreadLocalRng>());
        batchSize = kInitialBatchSize;
      }
    }

    const auto start = std::chrono::steady_clock::now();
    try {
      accumulator->accumulate(batchSize);
    } catch (const std::exception& e) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
      }
      emit runningChanged(false);
      emit errorOccurred(QString::fromUtf8(e.what()));
      continue;
    }
    const auto end = std::chrono::steady_clock::now();
    const double elapsedMs =
        (end - start) / std::chrono::duration<double, std::milli>(1.0);

    bool stale = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stale = (generation_ != localGen);
    }
    if (stale) {
      continue;
    }

    stagingBuffer = accumulator->histogram();

    bool committed = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (generation_ == localGen) {
        std::swap(committedBuffer_, stagingBuffer);
        committedPlottedIterations_ = accumulator->plottedIterations();
        ++snapshotVersion_;
        committed = true;
      }
    }

    if (committed) {
      emit batchCompleted();
    }

    if (elapsedMs > 0.0) {
      const double ratio = std::clamp(kTargetBatchMs / elapsedMs, 0.5, 2.0);
      batchSize = std::clamp(
          static_cast<uint64_t>(static_cast<double>(batchSize) * ratio),
          kMinBatchSize, kMaxBatchSize);
    } else {
      batchSize = (std::min)(batchSize * 2, kMaxBatchSize);
    }
  }
}

}  // namespace chaoskit::ui
