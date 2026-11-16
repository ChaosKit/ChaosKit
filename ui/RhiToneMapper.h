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

 private:
  bool ensureShadersBaked(QString* errorMessage);
  bool ensureSharedResources(QString* errorMessage);
  bool ensurePipeline(QRhiRenderPassDescriptor* renderPassDesc, int sampleCount,
                      QString* errorMessage);
  bool recreateHistogramTexture(const QSize& size);
  bool updateHistogramTexture(const core::HistogramBuffer* histogramToUpload,
                              QRhiResourceUpdateBatch* updates);

  QRhi* rhi_ = nullptr;
  QShader vertexShader_;
  QShader fragmentShader_;

  std::unique_ptr<QRhiBuffer> vertexBuffer_;
  std::unique_ptr<QRhiBuffer> uniformBuffer_;
  std::unique_ptr<QRhiSampler> sampler_;
  std::unique_ptr<QRhiTexture> histogramTexture_;
  std::unique_ptr<QRhiShaderResourceBindings> srb_;
  std::unique_ptr<QRhiGraphicsPipeline> pipeline_;
  QList<quint32> renderPassFormat_;
  int sampleCount_ = 1;
  bool vertexBufferUploaded_ = false;
  bool emptyHistogramUploaded_ = false;

  float gamma_ = 2.2f;
  float exposure_ = 0.0f;
  float vibrancy_ = 0.0f;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_RHITONEMAPPER_H
