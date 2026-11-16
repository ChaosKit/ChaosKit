#ifndef CHAOSKIT_UI_CPUACCUMULATORDRIVER_H
#define CHAOSKIT_UI_CPUACCUMULATORDRIVER_H

#include <QObject>
#include <QString>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>
#include "core/CameraSystem.h"
#include "core/ColorMap.h"
#include "core/HistogramBuffer.h"

namespace chaoskit::ui {

// Drives batched CPU particle accumulation on a dedicated worker thread while
// keeping System, ColorMap, and dimension swaps atomic against stale hits.
class CpuAccumulatorDriver : public QObject {
  Q_OBJECT
 public:
  explicit CpuAccumulatorDriver(QObject* parent = nullptr);
  ~CpuAccumulatorDriver() override;

  void setSystem(const core::CameraSystem& system);
  void clearSystem();
  void setColorMap(const core::ColorMap* colorMap);
  void setSize(uint32_t width, uint32_t height);
  void setRunning(bool running);
  void clear();

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

 signals:
  void batchCompleted();
  void runningChanged(bool running);
  void errorOccurred(const QString& message);

 private:
  void workerLoop();

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

  std::thread worker_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_CPUACCUMULATORDRIVER_H
