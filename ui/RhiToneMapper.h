#ifndef CHAOSKIT_UI_RHITONEMAPPER_H
#define CHAOSKIT_UI_RHITONEMAPPER_H

#include <rhi/qrhi.h>
#include <rhi/qshader.h>
#include <QList>
#include <QSize>
#include <QString>

#include <cstdint>
#include <memory>

#include "core/HistogramBuffer.h"

namespace chaoskit::ui {

// Cross-platform QRhi tone-mapping pass converting an accumulated RGBA32F
// histogram buffer into an RGBA8 render target using 100-IPP density
// normalization, logarithmic exposure scaling, gamma, and vibrancy.
class RhiToneMapper {
 public:
  RhiToneMapper() = default;
  ~RhiToneMapper();

  RhiToneMapper(const RhiToneMapper&) = delete;
  RhiToneMapper& operator=(const RhiToneMapper&) = delete;

  bool initialize(QRhi* rhi, QRhiRenderPassDescriptor* renderPassDesc,
                  int sampleCount = 1, QString* errorMessage = nullptr);
  void releaseResources();

  void setGamma(float gamma);
  void setExposure(float exposure);
  void setVibrancy(float vibrancy);

  // Records a histogram upload (if `histogramToUpload != nullptr`), updates the
  // tone-mapping uniform buffer with `100.0 / currentIpp` scaling, and executes
  // the full-screen tone-mapping render pass.
  void render(QRhiCommandBuffer* cb, QRhiRenderTarget* renderTarget,
              const core::HistogramBuffer* histogramToUpload,
              uint64_t plottedIterations,
              QRhiResourceUpdateBatch* endPassUpdates = nullptr);

  // Executes the full-screen tone-mapping render pass directly from
  // GPU-resident Q40.24 fixed-point HistogramSSBO and 64-bit CounterSSBO
  // buffers without any CPU histogram round-trips.
  void renderFromSsbo(QRhiCommandBuffer* cb, QRhiRenderTarget* renderTarget,
                      QRhiBuffer* histogramBuffer, QRhiBuffer* counterBuffer,
                      const QSize& histogramSize,
                      QRhiResourceUpdateBatch* endPassUpdates = nullptr);

  [[nodiscard]] static QShader bakeSsboVertexShader(
      QString* errorMessage = nullptr);
  [[nodiscard]] static QShader bakeSsboFragmentShader(
      QString* errorMessage = nullptr);

 private:
  bool ensureShadersBaked(QString* errorMessage);
  bool ensureSharedResources(QString* errorMessage);
  bool ensurePipeline(QRhiRenderPassDescriptor* renderPassDesc, int sampleCount,
                      QString* errorMessage);
  bool ensureSsboShadersBaked(QString* errorMessage);
  bool ensureSsboResources(QRhiRenderPassDescriptor* renderPassDesc,
                           int sampleCount, QRhiBuffer* histogramBuffer,
                           QRhiBuffer* counterBuffer, QString* errorMessage);
  bool recreateHistogramTexture(const QSize& size);
  bool updateHistogramTexture(const core::HistogramBuffer* histogramToUpload,
                              QRhiResourceUpdateBatch* updates);

  QRhi* rhi_ = nullptr;
  QShader vertexShader_;
  QShader fragmentShader_;
  QShader ssboVertexShader_;
  QShader ssboFragmentShader_;

  std::unique_ptr<QRhiBuffer> vertexBuffer_;
  std::unique_ptr<QRhiBuffer> uniformBuffer_;
  std::unique_ptr<QRhiBuffer> ssboUniformBuffer_;
  std::unique_ptr<QRhiSampler> sampler_;
  std::unique_ptr<QRhiTexture> histogramTexture_;
  std::unique_ptr<QRhiShaderResourceBindings> srb_;
  std::unique_ptr<QRhiShaderResourceBindings> ssboSrb_;
  std::unique_ptr<QRhiGraphicsPipeline> pipeline_;
  std::unique_ptr<QRhiGraphicsPipeline> ssboPipeline_;
  QRhiBuffer* ssboBoundHistogram_ = nullptr;
  QRhiBuffer* ssboBoundCounter_ = nullptr;
  QList<quint32> renderPassFormat_;
  QList<quint32> ssboRenderPassFormat_;
  int sampleCount_ = 1;
  int ssboSampleCount_ = 1;
  bool vertexBufferUploaded_ = false;
  bool emptyHistogramUploaded_ = false;

  float gamma_ = 2.2f;
  float exposure_ = 0.0f;
  float vibrancy_ = 0.0f;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_RHITONEMAPPER_H
