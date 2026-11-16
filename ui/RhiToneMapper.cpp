#include "RhiToneMapper.h"

#include <QByteArray>
#include <QColor>
#include <QMatrix4x4>

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "RhiShaderBaker.h"
#include "core/Color.h"
#include "core/HistogramBuffer.h"

namespace chaoskit::ui {

namespace {

constexpr float kQuadVertices[] = {
    -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,
};

struct ToneMapUniforms {
  float clipSpaceCorr[16];
  float gamma;
  float exposure;
  float vibrancy;
  float invDensityScale;
};

static_assert(sizeof(ToneMapUniforms) == 80);
static_assert(sizeof(core::Color) == 4 * sizeof(float));

constexpr char kToneMapVertexShader[] = R"GLSL(
#version 450

layout(location = 0) in vec2 pos;
layout(location = 0) out vec2 vUv;

layout(std140, binding = 0) uniform Params {
    mat4 clipSpaceCorr;
    float gamma;
    float exposure;
    float vibrancy;
    float invDensityScale;
} u;

void main() {
    vUv = pos * 0.5 + 0.5;
    gl_Position = u.clipSpaceCorr * vec4(pos, 0.0, 1.0);
}
)GLSL";

constexpr char kToneMapFragmentShader[] = R"GLSL(
#version 450

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Params {
    mat4 clipSpaceCorr;
    float gamma;
    float exposure;
    float vibrancy;
    float invDensityScale;
} u;

layout(binding = 1) uniform sampler2D histogramTex;

const float kLog10 = 2.302585092994046;

float logmap(float x) {
    return log(x + 1.0) / kLog10;
}

float safePow(float base, float exponent) {
    return base > 0.0 ? pow(base, exponent) : 0.0;
}

void main() {
    vec4 rawPoint = texture(histogramTex, vUv);
    if (u.invDensityScale <= 0.0 || rawPoint.w <= 0.0) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec4 point = rawPoint * u.invDensityScale;
    float intensity = point.w * exp(-u.exposure + 1.0);
    if (intensity <= 0.0) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float scale = logmap(intensity) / intensity;
    vec4 scaledPoint = point * scale;
    if (scaledPoint.w <= 0.0) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec4 vibrantPoint = vec4(
        safePow(scaledPoint.x, u.gamma),
        safePow(scaledPoint.y, u.gamma),
        safePow(scaledPoint.z, u.gamma),
        safePow(scaledPoint.w, u.gamma));
    float gammaScale = vibrantPoint.w / scaledPoint.w;
    vec4 nonvibrantPoint = scaledPoint * gammaScale;

    vec3 newColor =
        (nonvibrantPoint + (vibrantPoint - nonvibrantPoint) * u.vibrancy).xyz;
    fragColor = vec4(clamp(newColor, 0.0, 1.0), 1.0);
}
)GLSL";

}  // namespace

RhiToneMapper::~RhiToneMapper() { releaseResources(); }

bool RhiToneMapper::initialize(QRhi* rhi,
                               QRhiRenderPassDescriptor* renderPassDesc,
                               int sampleCount, QString* errorMessage) {
  if (rhi == nullptr || renderPassDesc == nullptr) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Invalid QRhi or render pass descriptor");
    }
    return false;
  }

  if (rhi_ != rhi) {
    releaseResources();
    rhi_ = rhi;
  }

  if (!ensureShadersBaked(errorMessage)) {
    return false;
  }
  if (!ensureSharedResources(errorMessage)) {
    return false;
  }
  return ensurePipeline(renderPassDesc, sampleCount, errorMessage);
}

void RhiToneMapper::releaseResources() {
  pipeline_.reset();
  srb_.reset();
  histogramTexture_.reset();
  sampler_.reset();
  uniformBuffer_.reset();
  vertexBuffer_.reset();
  renderPassFormat_.clear();
  sampleCount_ = 1;
  vertexBufferUploaded_ = false;
  emptyHistogramUploaded_ = false;
  rhi_ = nullptr;
}

void RhiToneMapper::setGamma(float gamma) { gamma_ = gamma; }

void RhiToneMapper::setExposure(float exposure) { exposure_ = exposure; }

void RhiToneMapper::setVibrancy(float vibrancy) { vibrancy_ = vibrancy; }

bool RhiToneMapper::ensureShadersBaked(QString* errorMessage) {
  if (vertexShader_.isValid() && fragmentShader_.isValid()) {
    return true;
  }

  vertexShader_ = RhiShaderBaker::bake(
      QShader::VertexStage, QByteArray(kToneMapVertexShader), errorMessage);
  if (!vertexShader_.isValid()) {
    return false;
  }

  fragmentShader_ = RhiShaderBaker::bake(
      QShader::FragmentStage, QByteArray(kToneMapFragmentShader), errorMessage);
  return fragmentShader_.isValid();
}

bool RhiToneMapper::recreateHistogramTexture(const QSize& size) {
  histogramTexture_.reset(rhi_->newTexture(QRhiTexture::RGBA32F, size));
  if (!histogramTexture_ || !histogramTexture_->create()) {
    histogramTexture_.reset();
    return false;
  }
  emptyHistogramUploaded_ = false;
  srb_->setBindings({
      QRhiShaderResourceBinding::uniformBuffer(
          0,
          QRhiShaderResourceBinding::VertexStage |
              QRhiShaderResourceBinding::FragmentStage,
          uniformBuffer_.get()),
      QRhiShaderResourceBinding::sampledTexture(
          1, QRhiShaderResourceBinding::FragmentStage, histogramTexture_.get(),
          sampler_.get()),
  });
  return srb_->create();
}

bool RhiToneMapper::ensureSharedResources(QString* errorMessage) {
  if (!vertexBuffer_) {
    vertexBuffer_.reset(rhi_->newBuffer(QRhiBuffer::Immutable,
                                        QRhiBuffer::VertexBuffer,
                                        sizeof(kQuadVertices)));
    if (!vertexBuffer_ || !vertexBuffer_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create quad vertex buffer");
      }
      return false;
    }
    vertexBufferUploaded_ = false;
  }

  if (!uniformBuffer_) {
    uniformBuffer_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic,
                                         QRhiBuffer::UniformBuffer,
                                         sizeof(ToneMapUniforms)));
    if (!uniformBuffer_ || !uniformBuffer_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage =
            QStringLiteral("Failed to create tone map uniform buffer");
      }
      return false;
    }
  }

  if (!sampler_) {
    sampler_.reset(rhi_->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest,
                                    QRhiSampler::None, QRhiSampler::ClampToEdge,
                                    QRhiSampler::ClampToEdge));
    if (!sampler_ || !sampler_->create()) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("Failed to create histogram sampler");
      }
      return false;
    }
  }

  if (!srb_) {
    srb_.reset(rhi_->newShaderResourceBindings());
    if (!srb_) {
      if (errorMessage != nullptr) {
        *errorMessage =
            QStringLiteral("Failed to allocate tone map resource bindings");
      }
      return false;
    }
  }

  if (!histogramTexture_ && !recreateHistogramTexture(QSize(1, 1))) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Failed to create histogram texture");
    }
    return false;
  }

  return true;
}

bool RhiToneMapper::ensurePipeline(QRhiRenderPassDescriptor* renderPassDesc,
                                   int sampleCount, QString* errorMessage) {
  const QList<quint32> rpFormat = renderPassDesc->serializedFormat();
  if (pipeline_ && sampleCount_ == sampleCount &&
      renderPassFormat_ == rpFormat) {
    return true;
  }

  pipeline_.reset(rhi_->newGraphicsPipeline());
  pipeline_->setShaderStages({
      {QRhiShaderStage::Vertex, vertexShader_},
      {QRhiShaderStage::Fragment, fragmentShader_},
  });

  QRhiVertexInputLayout inputLayout;
  inputLayout.setBindings({{2 * sizeof(float)}});
  inputLayout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float2, 0}});
  pipeline_->setVertexInputLayout(inputLayout);

  pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
  pipeline_->setCullMode(QRhiGraphicsPipeline::None);
  pipeline_->setTargetBlends({QRhiGraphicsPipeline::TargetBlend{}});
  pipeline_->setSampleCount(sampleCount);
  pipeline_->setShaderResourceBindings(srb_.get());
  pipeline_->setRenderPassDescriptor(renderPassDesc);

  if (!pipeline_->create()) {
    pipeline_.reset();
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Failed to create tone map pipeline");
    }
    return false;
  }

  renderPassFormat_ = rpFormat;
  sampleCount_ = sampleCount;
  return true;
}

bool RhiToneMapper::updateHistogramTexture(
    const core::HistogramBuffer* histogramToUpload,
    QRhiResourceUpdateBatch* updates) {
  const bool hasValidHistogram = histogramToUpload != nullptr &&
                                 histogramToUpload->width() > 0 &&
                                 histogramToUpload->height() > 0;

  if (hasValidHistogram) {
    const QSize desiredSize(static_cast<int>(histogramToUpload->width()),
                            static_cast<int>(histogramToUpload->height()));
    if (!histogramTexture_ || histogramTexture_->pixelSize() != desiredSize) {
      if (!recreateHistogramTexture(desiredSize)) {
        return false;
      }
    }

    const quint32 byteSize =
        static_cast<quint32>(histogramToUpload->size() * sizeof(core::Color));
    QRhiTextureSubresourceUploadDescription subres(histogramToUpload->data(),
                                                   byteSize);
    subres.setDataStride(
        static_cast<quint32>(histogramToUpload->width() * sizeof(core::Color)));
    updates->uploadTexture(
        histogramTexture_.get(),
        QRhiTextureUploadDescription(QRhiTextureUploadEntry(0, 0, subres)));
    return true;
  }

  if (!histogramTexture_ || histogramTexture_->pixelSize() != QSize(1, 1)) {
    if (!recreateHistogramTexture(QSize(1, 1))) {
      return false;
    }
  }

  if (!emptyHistogramUploaded_ || histogramToUpload != nullptr) {
    const core::Color zero = core::Color::zero();
    QRhiTextureSubresourceUploadDescription subres(&zero, sizeof(zero));
    subres.setDataStride(sizeof(zero));
    updates->uploadTexture(
        histogramTexture_.get(),
        QRhiTextureUploadDescription(QRhiTextureUploadEntry(0, 0, subres)));
    emptyHistogramUploaded_ = true;
  }

  return true;
}

void RhiToneMapper::render(QRhiCommandBuffer* cb,
                           QRhiRenderTarget* renderTarget,
                           const core::HistogramBuffer* histogramToUpload,
                           uint64_t plottedIterations,
                           QRhiResourceUpdateBatch* endPassUpdates) {
  if (rhi_ == nullptr || cb == nullptr || renderTarget == nullptr ||
      !pipeline_ || renderTarget->pixelSize().isEmpty()) {
    if (endPassUpdates != nullptr) {
      endPassUpdates->release();
    }
    return;
  }

  QRhiResourceUpdateBatch* updates = rhi_->nextResourceUpdateBatch();
  if (!vertexBufferUploaded_) {
    updates->uploadStaticBuffer(vertexBuffer_.get(), kQuadVertices);
    vertexBufferUploaded_ = true;
  }

  if (!updateHistogramTexture(histogramToUpload, updates)) {
    updates->release();
    if (endPassUpdates != nullptr) {
      endPassUpdates->release();
    }
    return;
  }

  const QSize targetSize = renderTarget->pixelSize();
  const QSize histSize =
      (histogramToUpload != nullptr && histogramToUpload->width() > 0 &&
       histogramToUpload->height() > 0)
          ? QSize(static_cast<int>(histogramToUpload->width()),
                  static_cast<int>(histogramToUpload->height()))
          : (histogramTexture_ && histogramTexture_->pixelSize() != QSize(1, 1)
                 ? histogramTexture_->pixelSize()
                 : targetSize);
  const uint64_t pixelCount =
      static_cast<uint64_t>((std::max)(0, histSize.width())) *
      static_cast<uint64_t>((std::max)(0, histSize.height()));
  const double currentIpp = pixelCount > 0
                                ? static_cast<double>(plottedIterations) /
                                      static_cast<double>(pixelCount)
                                : 0.0;
  const float invDensityScale =
      currentIpp > 0.0 ? static_cast<float>(100.0 / currentIpp) : 0.0f;

  ToneMapUniforms uniforms = {};
  const QMatrix4x4 corr = rhi_->clipSpaceCorrMatrix();
  std::memcpy(uniforms.clipSpaceCorr, corr.constData(),
              sizeof(uniforms.clipSpaceCorr));
  uniforms.gamma = gamma_;
  uniforms.exposure = exposure_;
  uniforms.vibrancy = vibrancy_;
  uniforms.invDensityScale = invDensityScale;

  updates->updateDynamicBuffer(uniformBuffer_.get(), 0, sizeof(uniforms),
                               &uniforms);

  cb->beginPass(renderTarget, QColor(0, 0, 0, 255), {1.0f, 0}, updates);
  cb->setGraphicsPipeline(pipeline_.get());
  cb->setViewport(QRhiViewport(0.0f, 0.0f,
                               static_cast<float>(targetSize.width()),
                               static_cast<float>(targetSize.height())));
  cb->setShaderResources(srb_.get());
  const QRhiCommandBuffer::VertexInput vbufBinding(vertexBuffer_.get(), 0);
  cb->setVertexInput(0, 1, &vbufBinding);
  cb->draw(6);
  cb->endPass(endPassUpdates);
}

}  // namespace chaoskit::ui
