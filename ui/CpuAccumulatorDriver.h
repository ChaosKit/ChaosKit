#ifndef CHAOSKIT_UI_CPUACCUMULATORDRIVER_H
#define CHAOSKIT_UI_CPUACCUMULATORDRIVER_H

#include <QObject>
#include <QString>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include "core/CameraSystem.h"
#include "core/ColorMap.h"
#include "core/HistogramBuffer.h"

namespace chaoskit::ui {

// Drives batched CPU particle accumulation across a pool of persistent worker
// threads while keeping System, ColorMap, and dimension swaps atomic against
// stale hits.
class CpuAccumulatorDriver : public QObject {
  Q_OBJECT
 public:
  explicit CpuAccumulatorDriver(QObject* parent = nullptr,
                                unsigned int threadCount = 0);
  ~CpuAccumulatorDriver() override;

  [[nodiscard]] unsigned int threadCount() const { return threadCount_; }
  [[nodiscard]] bool isRunning() const;
  [[nodiscard]] uint64_t plottedIterations() const;

  // Copies the committed histogram (and optional plotted-iteration count) into
  // `bufferOut` if the snapshot version advanced since `lastVersion`.
  bool syncSnapshotIfChanged(uint64_t& lastVersion,
                             core::HistogramBuffer& bufferOut,
                             uint64_t* plottedIterationsOut = nullptr) const;

  // Unconditionally copies the current committed histogram and plotted count.
  void snapshot(core::HistogramBuffer& bufferOut,
                uint64_t* plottedIterationsOut = nullptr) const;

  void setSystem(const core::CameraSystem& system);
  void clearSystem();
  void setColorMap(const core::ColorMap* colorMap);
  void setSize(uint32_t width, uint32_t height);
  void setRunning(bool running);
  void clear();

 signals:
  void batchCompleted();
  void runningChanged(bool running);
  void errorOccurred(const QString& message);

 private:
  struct WorkerSlot;

  static uint64_t mergeWorkerHistograms(
      const std::vector<std::unique_ptr<WorkerSlot>>& workerSlots,
      core::HistogramBuffer& stagingBuffer);
  void coordinatorLoop();
  void workerLoop(size_t workerIndex);

  const unsigned int threadCount_;

  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::optional<core::CameraSystem> system_ = std::nullopt;
  const core::ColorMap* colorMap_ = nullptr;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  bool running_ = false;
  bool stopRequested_ = false;
  uint64_t generation_ = 1;
  uint64_t snapshotVersion_ = 1;
  core::HistogramBuffer committedBuffer_;
  uint64_t committedPlottedIterations_ = 0;

  std::mutex poolMutex_;
  std::condition_variable poolWorkCv_;
  std::condition_variable poolDoneCv_;
  std::optional<core::CameraSystem> batchSystem_ = std::nullopt;
  const core::ColorMap* batchColorMap_ = nullptr;
  uint32_t batchWidth_ = 0;
  uint32_t batchHeight_ = 0;
  uint64_t batchGeneration_ = 0;
  bool poolStopRequested_ = false;
  size_t remainingWorkers_ = 0;
  std::vector<std::unique_ptr<WorkerSlot>> workerSlots_;
  std::vector<std::thread> workers_;
  std::thread coordinator_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_CPUACCUMULATORDRIVER_H
