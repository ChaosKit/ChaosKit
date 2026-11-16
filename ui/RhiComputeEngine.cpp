#include "RhiComputeEngine.h"

#include <QByteArray>
#include <QSize>
#include <QString>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include "RhiShaderBaker.h"
#include "core/CameraSystem.h"
#include "core/Color.h"
#include "core/ColorMap.h"
#include "core/GpuSystemCompiler.h"

namespace chaoskit::ui {

namespace {

constexpr uint32_t kWorkgroupSize = core::GpuSystemCompiler::kWorkgroupSize;
constexpr uint32_t kParticleStructBytes = 32;
constexpr uint32_t kHistogramPixelWords = 8;
constexpr uint32_t kCounterBytes = 16;
constexpr uint32_t kMaxWorkgroupsX = 65535;

struct ClearUniforms {
  uint32_t totalPixels;
  uint32_t gridStride;
  uint32_t pad0;
  uint32_t pad1;
};

static_assert(sizeof(ClearUniforms) == 16);
static_assert(sizeof(core::Color) == 4 * sizeof(float));

constexpr char kClearComputeShader[] = R"GLSL(#version 450

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0) uniform ClearParams {
    uint totalPixels;
    uint gridStride;
    uint pad0;
    uint pad1;
} u;

layout(std430, binding = 1) buffer HistogramBuffer {
    uint words[];
} histogram;

layout(std430, binding = 2) buffer CounterBuffer {
    uint plottedLo;
    uint plottedHi;
    uint pad0;
    uint pad1;
} counter;

void main() {
    uint gid = gl_GlobalInvocationID.x;
    for (uint px = gid; px < u.totalPixels; px += u.gridStride) {
        uint base = px * 8u;
        histogram.words[base + 0u] = 0u;
        histogram.words[base + 1u] = 0u;
        histogram.words[base + 2u] = 0u;
        histogram.words[base + 3u] = 0u;
        histogram.words[base + 4u] = 0u;
        histogram.words[base + 5u] = 0u;
        histogram.words[base + 6u] = 0u;
        histogram.words[base + 7u] = 0u;
    }
    if (gid == 0u) {
        counter.plottedLo = 0u;
        counter.plottedHi = 0u;
    }
}
)GLSL";

constexpr char kInitComputeShader[] = R"GLSL(#version 450

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct GpuParticle {
    vec4 posColor;
    ivec4 lifeWarmupRng;
};

layout(std140, binding = 0) uniform InitParams {
    uint width;
    uint height;
    int minLifetime;
    int maxLifetime;
    uint particleCount;
    uint ticksPerInvocation;
    uint seededMode;
    uint reserved0;
    uint seedLo;
    uint seedHi;
    uint tickOffsetLo;
    uint tickOffsetHi;
    float halfMax;
    float halfWidth;
    float halfHeight;
    float reserved1;
} u;

layout(std430, binding = 1) buffer ParticleBuffer {
    GpuParticle particles[];
} particlePool;

void ck_umul32(uint a, uint b, out uint hi, out uint lo) {
    uint a0 = a & 0xffffu;
    uint a1 = a >> 16u;
    uint b0 = b & 0xffffu;
    uint b1 = b >> 16u;
    uint p00 = a0 * b0;
    uint p01 = a0 * b1;
    uint p10 = a1 * b0;
    uint p11 = a1 * b1;
    uint mid = (p00 >> 16u) + (p01 & 0xffffu) + (p10 & 0xffffu);
    lo = (p00 & 0xffffu) | ((mid & 0xffffu) << 16u);
    hi = p11 + (p01 >> 16u) + (p10 >> 16u) + (mid >> 16u);
}

uvec4 ck_philox4x32(uvec4 ctr, uvec2 key) {
    for (int r = 0; r < 10; ++r) {
        uint hi0;
        uint lo0;
        uint hi1;
        uint lo1;
        ck_umul32(0xd2511f53u, ctr.x, hi0, lo0);
        ck_umul32(0xcd9e8d57u, ctr.z, hi1, lo1);
        ctr = uvec4(hi1 ^ ctr.y ^ key.x, lo1, hi0 ^ ctr.w ^ key.y, lo0);
        key.x += 0x9e3779b9u;
        key.y += 0xbb67ae85u;
    }
    return ctr;
}

void main() {
    uint gid = gl_GlobalInvocationID.x;
    if (gid >= u.particleCount) {
        return;
    }

    uvec4 r = ck_philox4x32(
        uvec4(gid, 0xffffffffu, 0xffffffffu, 0u),
        uvec2(u.seedLo, u.seedHi));

    float x = (float(r.x >> 8u) * 5.9604644775390625e-8) * 2.0 - 1.0;
    float y = (float(r.y >> 8u) * 5.9604644775390625e-8) * 2.0 - 1.0;
    float color = float(r.z >> 8u) * 5.9604644775390625e-8;

    int lifetime = -1;
    if (u.maxLifetime >= 0) {
        int minInit = min(u.minLifetime + 1, u.maxLifetime);
        int span = max(1, u.maxLifetime - minInit + 1);
        float u01 = float(r.w >> 8u) * 5.9604644775390625e-8;
        lifetime = minInit + clamp(int(floor(u01 * float(span))), 0, span - 1);
    }

    particlePool.particles[gid].posColor = vec4(x, y, color, 0.0);
    particlePool.particles[gid].lifeWarmupRng =
        ivec4(lifetime, u.minLifetime, 0, 0);
}
)GLSL";

uint64_t generateEntropySeed() {
  std::random_device rd;
  const uint64_t lo = static_cast<uint64_t>(rd());
  const uint64_t hi = static_cast<uint64_t>(rd());
  const uint64_t ticks = static_cast<uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch() /
      std::chrono::nanoseconds(1));
  return (hi << 32u) ^ lo ^ (ticks * 0x9e3779b97f4a7c15ULL);
}

std::vector<core::Color> sampleColorMapLut(const core::ColorMap* colorMap) {
  std::vector<core::Color> lut(RhiComputeEngine::kColorMapLutWidth);
  for (uint32_t i = 0; i < RhiComputeEngine::kColorMapLutWidth; ++i) {
    const float t = static_cast<float>(i) /
                    static_cast<float>(RhiComputeEngine::kColorMapLutWidth - 1);
    if (colorMap != nullptr) {
      lut[i] = colorMap->map(t);
    } else {
      lut[i] = core::Color{1.0f, 1.0f, 1.0f, t};
    }
  }
  return lut;
}

}  // namespace

RhiComputeEngine::~RhiComputeEngine() { releaseResources(); }

QByteArray RhiComputeEngine::clearShaderSource() {
  return QByteArray(kClearComputeShader);
}

QByteArray RhiComputeEngine::initShaderSource() {
  return QByteArray(kInitComputeShader);
}

bool RhiComputeEngine::initialize(QRhi* rhi, QString* errorMessage) {
  if (rhi == nullptr) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Invalid QRhi pointer");
    }
    return false;
  }
  if (!rhi->isFeatureSupported(QRhi::Compute)) {
    if (errorMessage != nullptr) {
      *errorMessage =
          QStringLiteral("Active QRhi backend lacks Compute support");
    }
    return false;
  }

  if (rhi_ != rhi) {
    releaseResources();
    rhi_ = rhi;
  }

  if (!ensureSharedResources(core::GpuSystemCompiler::kHeaderByteSize + 16u,
                             errorMessage)) {
    return false;
  }
  if (!ensureBuiltinPipelines(errorMessage)) {
    return false;
  }
  refreshActiveSeed();
  return true;
}

void RhiComputeEngine::releaseResources() {
  pipelineCache_.clear();
  initPipeline_.reset();
  clearPipeline_.reset();
  initSrb_.reset();
  clearSrb_.reset();
  colorMapSampler_.reset();
  colorMapTexture_.reset();
  counterBuffer_.reset();
  histogramBuffer_.reset();
  particleBuffer_.reset();
  uniformBuffer_.reset();
  clearUniformBuffer_.reset();
  activeStructuralKey_.clear();
  needsReset_ = true;
  colorMapDirty_ = true;
  particlesDirty_ = true;
  histogramDirty_ = true;
  resourceEpoch_ = 0;
  clearBoundEpoch_ = 0;
  initBoundEpoch_ = 0;
  rhi_ = nullptr;
}

bool RhiComputeEngine::setSystem(const core::CameraSystem& system,
                                 QString* errorMessage) {
  std::string key;
  try {
    key = core::GpuSystemCompiler::computeStructuralKey(system);
  } catch (const std::exception& e) {
    if (errorMessage != nullptr) {
      *errorMessage = QString::fromUtf8(e.what());
    }
    return false;
  }

  if (pipelineCache_.find(key) == pipelineCache_.end()) {
    core::CompiledGpuSystem compiled;
    try {
      compiled = core::GpuSystemCompiler::compile(system);
    } catch (const std::exception& e) {
      if (errorMessage != nullptr) {
        *errorMessage = QString::fromUtf8(e.what());
      }
      return false;
    }

    QString bakeError;
    QShader shader = RhiShaderBaker::bake(
        QShader::ComputeStage,
        QByteArray::fromStdString(compiled.computeShaderSource), &bakeError);
    if (!shader.isValid()) {
      if (errorMessage != nullptr) {
        *errorMessage = bakeError.isEmpty()
                            ? QStringLiteral("Failed to bake compute shader")
                            : bakeError;
      }
      return false;
    }

    CachedPipeline entry;
    entry.shader = std::move(shader);
    entry.layout = std::move(compiled.layout);

    if (rhi_ != nullptr) {
      if (!ensureSharedResources(entry.layout.uniformBufferSize,
                                 errorMessage)) {
        return false;
      }
      entry.srb.reset(rhi_->newShaderResourceBindings());
      if (!entry.srb || !ensurePipelineBindings(entry, errorMessage)) {
        return false;
      }
      entry.pipeline.reset(rhi_->newComputePipeline());
      if (!entry.pipeline) {
        if (errorMessage != nullptr) {
          *errorMessage = QStringLiteral("Failed to allocate compute pipeline");
        }
        return false;
      }
      entry.pipeline->setShaderStage({QRhiShaderStage::Compute, entry.shader});
      entry.pipeline->setShaderResourceBindings(entry.srb.get());
      if (!entry.pipeline->create()) {
        if (errorMessage != nullptr) {
          *errorMessage = QStringLiteral("Failed to create compute pipeline");
        }
        return false;
      }
    }

    pipelineCache_.emplace(key, std::move(entry));
    ++compileCount_;
  }

  system_ = system;
  activeStructuralKey_ = std::move(key);
  reset();
  return true;
}

void RhiComputeEngine::setColorMap(const core::ColorMap* colorMap) {
  if (colorMap_ == colorMap) {
    return;
  }
  colorMap_ = colorMap;
  colorMapDirty_ = true;
  reset();
}

void RhiComputeEngine::setSize(const QSize& size) {
  const QSize normalized((std::max)(1, size.width()),
                         (std::max)(1, size.height()));
  if (size_ == normalized) {
    return;
  }
  size_ = normalized;
  histogramDirty_ = true;
  reset();
}

void RhiComputeEngine::setParticleCount(uint32_t particleCount) {
  const uint32_t clamped = (std::max)(kWorkgroupSize, particleCount);
  const uint32_t rounded =
      ((clamped + kWorkgroupSize - 1u) / kWorkgroupSize) * kWorkgroupSize;
  if (particleCount_ == rounded) {
    return;
  }
  particleCount_ = rounded;
  particlesDirty_ = true;
  reset();
}

void RhiComputeEngine::setSeed(std::optional<uint64_t> seed) {
  seededMode_ = seed.has_value();
  configuredSeed_ = seed.value_or(0);
  reset();
}

void RhiComputeEngine::reset() {
  needsReset_ = true;
  tickOffset_ = 0;
  refreshActiveSeed();
}

void RhiComputeEngine::refreshActiveSeed() {
  activeSeed_ = seededMode_ ? configuredSeed_ : generateEntropySeed();
}

bool RhiComputeEngine::ensureBuiltinPipelines(QString* errorMessage) {
  if (!clearShader_.isValid()) {
    QString bakeError;
    clearShader_ = RhiShaderBaker::bake(QShader::ComputeStage,
                                        clearShaderSource(), &bakeError);
    if (!clearShader_.isValid()) {
      if (errorMessage != nullptr) {
        *errorMessage = bakeError.isEmpty()
                            ? QStringLiteral("Failed to bake clear shader")
                            : bakeError;
      }
      return false;
    }
  }

  if (!initShader_.isValid()) {
    QString bakeError;
    initShader_ = RhiShaderBaker::bake(QShader::ComputeStage,
                                       initShaderSource(), &bakeError);
    if (!initShader_.isValid()) {
      if (errorMessage != nullptr) {
        *errorMessage = bakeError.isEmpty()
                            ? QStringLiteral("Failed to bake init shader")
                            : bakeError;
      }
      return false;
    }
  }

  if (!clearSrb_) {
    clearSrb_.reset(rhi_->newShaderResourceBindings());
    clearBoundEpoch_ = 0;
  }
  if (!initSrb_) {
    initSrb_.reset(rhi_->newShaderResourceBindings());
    initBoundEpoch_ = 0;
  }
  if (!clearSrb_ || !initSrb_) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Failed to allocate compute SRBs");
    }
    return false;
  }

  if (clearBoundEpoch_ != resourceEpoch_) {
    clearSrb_->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(
            0, QRhiShaderResourceBinding::ComputeStage,
            clearUniformBuffer_.get()),
        QRhiShaderResourceBinding::bufferLoadStore(
            1, QRhiShaderResourceBinding::ComputeStage, histogramBuffer_.get()),
        QRhiShaderResourceBinding::bufferLoadStore(
            2, QRhiShaderResourceBinding::ComputeStage, counterBuffer_.get()),
    });
    if (!clearSrb_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create clear SRB");
      }
      return false;
    }
    clearBoundEpoch_ = resourceEpoch_;
  }

  if (initBoundEpoch_ != resourceEpoch_) {
    initSrb_->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(
            0, QRhiShaderResourceBinding::ComputeStage, uniformBuffer_.get()),
        QRhiShaderResourceBinding::bufferLoadStore(
            1, QRhiShaderResourceBinding::ComputeStage, particleBuffer_.get()),
    });
    if (!initSrb_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create init SRB");
      }
      return false;
    }
    initBoundEpoch_ = resourceEpoch_;
  }

  if (!clearPipeline_) {
    clearPipeline_.reset(rhi_->newComputePipeline());
    clearPipeline_->setShaderStage({QRhiShaderStage::Compute, clearShader_});
    clearPipeline_->setShaderResourceBindings(clearSrb_.get());
    if (!clearPipeline_->create()) {
      clearPipeline_.reset();
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create clear pipeline");
      }
      return false;
    }
  }

  if (!initPipeline_) {
    initPipeline_.reset(rhi_->newComputePipeline());
    initPipeline_->setShaderStage({QRhiShaderStage::Compute, initShader_});
    initPipeline_->setShaderResourceBindings(initSrb_.get());
    if (!initPipeline_->create()) {
      initPipeline_.reset();
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create init pipeline");
      }
      return false;
    }
  }

  return true;
}

bool RhiComputeEngine::ensureSharedResources(uint32_t requiredUniformSize,
                                             QString* errorMessage) {
  bool resourceRecreated = false;

  if (!clearUniformBuffer_) {
    clearUniformBuffer_.reset(rhi_->newBuffer(
        QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(ClearUniforms)));
    if (!clearUniformBuffer_ || !clearUniformBuffer_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create clear uniform buffer");
      }
      return false;
    }
    resourceRecreated = true;
  }

  const quint32 alignedUbufSize =
      static_cast<quint32>(rhi_->ubufAligned(static_cast<int>(
          (std::max)(requiredUniformSize,
                     core::GpuSystemCompiler::kHeaderByteSize + 16u))));
  if (!uniformBuffer_ || uniformBuffer_->size() < alignedUbufSize) {
    uniformBuffer_.reset(rhi_->newBuffer(
        QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, alignedUbufSize));
    if (!uniformBuffer_ || !uniformBuffer_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage =
            QStringLiteral("Failed to create compute uniform buffer");
      }
      return false;
    }
    resourceRecreated = true;
  }

  const uint64_t particleBytes =
      static_cast<uint64_t>(particleCount_) * kParticleStructBytes;
  if (particleBytes > (std::numeric_limits<quint32>::max)()) {
    if (errorMessage != nullptr) {
      *errorMessage =
          QStringLiteral("Particle buffer size exceeds 32-bit limit");
    }
    return false;
  }
  if (!particleBuffer_ || particlesDirty_ ||
      particleBuffer_->size() != static_cast<quint32>(particleBytes)) {
    particleBuffer_.reset(rhi_->newBuffer(QRhiBuffer::Static,
                                          QRhiBuffer::StorageBuffer,
                                          static_cast<quint32>(particleBytes)));
    if (!particleBuffer_ || !particleBuffer_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create particle SSBO");
      }
      return false;
    }
    particlesDirty_ = false;
    resourceRecreated = true;
  }

  const uint64_t pixelCount =
      static_cast<uint64_t>((std::max)(1, size_.width())) *
      static_cast<uint64_t>((std::max)(1, size_.height()));
  const uint64_t histogramBytes =
      pixelCount * kHistogramPixelWords * sizeof(uint32_t);
  if (histogramBytes > (std::numeric_limits<quint32>::max)()) {
    if (errorMessage != nullptr) {
      *errorMessage =
          QStringLiteral("Histogram SSBO size exceeds 32-bit limit");
    }
    return false;
  }
  if (!histogramBuffer_ || histogramDirty_ ||
      histogramBuffer_->size() != static_cast<quint32>(histogramBytes)) {
    histogramBuffer_.reset(
        rhi_->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer,
                        static_cast<quint32>(histogramBytes)));
    if (!histogramBuffer_ || !histogramBuffer_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create histogram SSBO");
      }
      return false;
    }
    histogramDirty_ = false;
    resourceRecreated = true;
  }

  if (!counterBuffer_) {
    counterBuffer_.reset(rhi_->newBuffer(
        QRhiBuffer::Static, QRhiBuffer::StorageBuffer, kCounterBytes));
    if (!counterBuffer_ || !counterBuffer_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create counter SSBO");
      }
      return false;
    }
    resourceRecreated = true;
  }

  if (!colorMapTexture_) {
    colorMapTexture_.reset(rhi_->newTexture(
        QRhiTexture::RGBA32F, QSize(static_cast<int>(kColorMapLutWidth), 1)));
    if (!colorMapTexture_ || !colorMapTexture_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create ColorMap LUT texture");
      }
      return false;
    }
    colorMapDirty_ = true;
    resourceRecreated = true;
  }

  if (!colorMapSampler_) {
    colorMapSampler_.reset(rhi_->newSampler(
        QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
        QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
    if (!colorMapSampler_ || !colorMapSampler_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create ColorMap LUT sampler");
      }
      return false;
    }
    resourceRecreated = true;
  }

  if (resourceRecreated || resourceEpoch_ == 0) {
    ++resourceEpoch_;
  }
  return true;
}

bool RhiComputeEngine::ensurePipelineBindings(CachedPipeline& entry,
                                              QString* errorMessage) {
  if (!entry.srb) {
    entry.srb.reset(rhi_->newShaderResourceBindings());
    entry.boundEpoch = 0;
  }
  if (!entry.srb) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Failed to allocate compute pipeline SRB");
    }
    return false;
  }
  if (entry.boundEpoch == resourceEpoch_) {
    return true;
  }

  entry.srb->setBindings({
      QRhiShaderResourceBinding::uniformBuffer(
          0, QRhiShaderResourceBinding::ComputeStage, uniformBuffer_.get()),
      QRhiShaderResourceBinding::bufferLoadStore(
          1, QRhiShaderResourceBinding::ComputeStage, particleBuffer_.get()),
      QRhiShaderResourceBinding::bufferLoadStore(
          2, QRhiShaderResourceBinding::ComputeStage, histogramBuffer_.get()),
      QRhiShaderResourceBinding::bufferLoadStore(
          3, QRhiShaderResourceBinding::ComputeStage, counterBuffer_.get()),
      QRhiShaderResourceBinding::sampledTexture(
          4, QRhiShaderResourceBinding::ComputeStage, colorMapTexture_.get(),
          colorMapSampler_.get()),
  });
  if (!entry.srb->create()) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Failed to create compute pipeline SRB");
    }
    return false;
  }
  entry.boundEpoch = resourceEpoch_;
  return true;
}

bool RhiComputeEngine::dispatch(QRhiCommandBuffer* cb,
                                uint32_t ticksPerInvocation,
                                QString* errorMessage) {
  if (rhi_ == nullptr || cb == nullptr || !system_.has_value()) {
    if (errorMessage != nullptr) {
      *errorMessage =
          QStringLiteral("Compute engine not initialized or system not set");
    }
    return false;
  }
  if (ticksPerInvocation == 0) {
    return true;
  }

  auto cacheIt = pipelineCache_.find(activeStructuralKey_);
  if (cacheIt == pipelineCache_.end()) {
    if (errorMessage != nullptr) {
      *errorMessage =
          QStringLiteral("Compiled pipeline missing from compute cache");
    }
    return false;
  }

  CachedPipeline& cached = cacheIt->second;
  if (!ensureSharedResources(cached.layout.uniformBufferSize, errorMessage)) {
    return false;
  }
  if (!ensureBuiltinPipelines(errorMessage)) {
    return false;
  }
  if (!ensurePipelineBindings(cached, errorMessage)) {
    return false;
  }
  if (!cached.pipeline) {
    cached.pipeline.reset(rhi_->newComputePipeline());
    cached.pipeline->setShaderStage({QRhiShaderStage::Compute, cached.shader});
    cached.pipeline->setShaderResourceBindings(cached.srb.get());
    if (!cached.pipeline->create()) {
      cached.pipeline.reset();
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create compute pipeline");
      }
      return false;
    }
  }

  std::vector<uint8_t> uniformBytes;
  try {
    const core::GpuDispatchConfig config{
        static_cast<uint32_t>(size_.width()),
        static_cast<uint32_t>(size_.height()),
        particleCount_,
        ticksPerInvocation,
        seededMode_,
        activeSeed_,
        tickOffset_,
    };
    uniformBytes =
        core::GpuSystemCompiler::packUniforms(cached.layout, *system_, config);
  } catch (const std::exception& e) {
    if (errorMessage != nullptr) {
      *errorMessage = QString::fromUtf8(e.what());
    }
    return false;
  }

  QRhiResourceUpdateBatch* updates = rhi_->nextResourceUpdateBatch();
  updates->updateDynamicBuffer(uniformBuffer_.get(), 0,
                               static_cast<quint32>(uniformBytes.size()),
                               uniformBytes.data());

  if (colorMapDirty_) {
    const std::vector<core::Color> lut = sampleColorMapLut(colorMap_);
    const quint32 byteSize =
        static_cast<quint32>(lut.size() * sizeof(core::Color));
    QRhiTextureSubresourceUploadDescription subres(lut.data(), byteSize);
    subres.setDataStride(byteSize);
    updates->uploadTexture(
        colorMapTexture_.get(),
        QRhiTextureUploadDescription(QRhiTextureUploadEntry(0, 0, subres)));
    colorMapDirty_ = false;
  }

  if (needsReset_) {
    const uint64_t totalPixels64 = static_cast<uint64_t>(size_.width()) *
                                   static_cast<uint64_t>(size_.height());
    const uint32_t totalPixels = static_cast<uint32_t>(totalPixels64);
    const uint32_t clearGroups =
        (std::min)(kMaxWorkgroupsX,
                   (std::max)(1u, (totalPixels + kWorkgroupSize - 1u) /
                                      kWorkgroupSize));
    const ClearUniforms clearUniforms{
        totalPixels,
        clearGroups * kWorkgroupSize,
        0u,
        0u,
    };
    updates->updateDynamicBuffer(clearUniformBuffer_.get(), 0,
                                 sizeof(clearUniforms), &clearUniforms);

    const int particleGroups =
        static_cast<int>(particleCount_ / kWorkgroupSize);
    cb->beginComputePass(updates);
    cb->setComputePipeline(clearPipeline_.get());
    cb->setShaderResources(clearSrb_.get());
    cb->dispatch(static_cast<int>(clearGroups), 1, 1);

    cb->setComputePipeline(initPipeline_.get());
    cb->setShaderResources(initSrb_.get());
    cb->dispatch(particleGroups, 1, 1);
    cb->endComputePass();

    updates = nullptr;
    needsReset_ = false;
  }

  const int particleGroups = static_cast<int>(particleCount_ / kWorkgroupSize);
  cb->beginComputePass(updates);
  cb->setComputePipeline(cached.pipeline.get());
  cb->setShaderResources(cached.srb.get());
  cb->dispatch(particleGroups, 1, 1);
  cb->endComputePass();

  tickOffset_ += ticksPerInvocation;
  return true;
}

}  // namespace chaoskit::ui
