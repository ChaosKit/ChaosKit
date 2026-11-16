#include "CpuAccumulator.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include "CameraSystem.h"
#include "Color.h"
#include "ColorMap.h"
#include "Rng.h"
#include "ThreadLocalRng.h"

namespace chaoskit::core {

namespace {

std::shared_ptr<Rng> resolveRng(std::shared_ptr<Rng> rng) {
  return rng ? std::move(rng) : std::make_shared<ThreadLocalRng>();
}

}  // namespace

CpuAccumulator::CpuAccumulator(const CameraSystem& system, uint32_t width,
                               uint32_t height, const ColorMap* colorMap,
                               std::shared_ptr<Rng> rng)
    : processor_(system, resolveRng(std::move(rng))),
      particle_(processor_.createParticle()),
      colorMap_(colorMap),
      buffer_(width, height) {}

void CpuAccumulator::setSystem(const CameraSystem& system) {
  processor_.setCameraSystem(system);
  reset();
}

void CpuAccumulator::setColorMap(const ColorMap* colorMap) {
  if (colorMap_ == colorMap) {
    return;
  }
  colorMap_ = colorMap;
  reset();
}

void CpuAccumulator::setSize(uint32_t width, uint32_t height) {
  if (width == this->width() && height == this->height()) {
    return;
  }
  buffer_.resize(width, height);
  plottedIterations_ = 0;
  particle_ = processor_.createParticle();
}

void CpuAccumulator::reset() {
  buffer_.clear();
  plottedIterations_ = 0;
  particle_ = processor_.createParticle();
}

uint64_t CpuAccumulator::accumulate(uint64_t stepCount) {
  if (stepCount == 0 || buffer_.width() == 0 || buffer_.height() == 0) {
    return 0;
  }

  const auto width = static_cast<uint32_t>(buffer_.width());
  const auto height = static_cast<uint32_t>(buffer_.height());
  const float halfMax = 0.5f * static_cast<float>((std::max)(width, height));
  const float halfWidth = 0.5f * static_cast<float>(width);
  const float halfHeight = 0.5f * static_cast<float>(height);
  const auto dWidth = static_cast<double>(width);
  const auto dHeight = static_cast<double>(height);

  uint64_t batchPlotted = 0;
  for (uint64_t i = 0; i < stepCount; ++i) {
    if (!processor_.step(particle_)) {
      continue;
    }

    const SystemParticle output = processor_.processCamera(particle_);
    if (!std::isfinite(output.x()) || !std::isfinite(output.y()) ||
        !std::isfinite(output.color)) {
      continue;
    }

    ++batchPlotted;

    const float fx = output.x() * halfMax + halfWidth;
    const float fy = output.y() * halfMax + halfHeight;
    if (fx >= 0.0f && fy >= 0.0f && static_cast<double>(fx) < dWidth &&
        static_cast<double>(fy) < dHeight) {
      const auto ix = static_cast<size_t>(fx);
      const auto iy = static_cast<size_t>(fy);
      if (ix < buffer_.width() && iy < buffer_.height()) {
        Color* entry = buffer_(ix, iy);
        if (colorMap_ != nullptr) {
          *entry += colorMap_->map(std::clamp(output.color, 0.0f, 1.0f));
        } else {
          *entry += Color{1.0f, 1.0f, 1.0f, output.color};
        }
      }
    }
  }

  plottedIterations_ += batchPlotted;
  return batchPlotted;
}

uint64_t CpuAccumulator::accumulatePlotted(uint64_t targetPlotted) {
  if (targetPlotted == 0 || buffer_.width() == 0 || buffer_.height() == 0) {
    return 0;
  }

  const uint64_t minWarmup =
      static_cast<uint64_t>((std::max)(0, processor_.lifetime().min()));
  const uint64_t maxConsecutiveZero =
      (std::max)(uint64_t{65536}, (minWarmup + 2) * uint64_t{1024});

  uint64_t totalPlotted = 0;
  uint64_t consecutiveZero = 0;
  while (totalPlotted < targetPlotted) {
    const uint64_t remaining = targetPlotted - totalPlotted;
    const uint64_t chunk = (std::min)(remaining, maxConsecutiveZero);
    const uint64_t got = accumulate(chunk);
    totalPlotted += got;
    if (got == 0) {
      consecutiveZero += chunk;
      if (consecutiveZero >= maxConsecutiveZero) {
        break;
      }
    } else {
      consecutiveZero = 0;
    }
  }
  return totalPlotted;
}

void CpuAccumulator::merge(const CpuAccumulator& other) {
  if (buffer_.width() != other.buffer_.width() ||
      buffer_.height() != other.buffer_.height()) {
    throw std::invalid_argument("Histogram dimensions must match for merge");
  }
  Color* dst = buffer_.data();
  const Color* src = other.buffer_.data();
  const size_t count = buffer_.size();
  for (size_t i = 0; i < count; ++i) {
    dst[i] += src[i];
  }
  plottedIterations_ += other.plottedIterations_;
}

double CpuAccumulator::iterationsPerPixel() const {
  if (buffer_.size() == 0) {
    return 0.0;
  }
  return static_cast<double>(plottedIterations_) /
         static_cast<double>(buffer_.size());
}

}  // namespace chaoskit::core
