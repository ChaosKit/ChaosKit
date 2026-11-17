#ifndef CHAOSKIT_UI_RHISHADERBAKER_H
#define CHAOSKIT_UI_RHISHADERBAKER_H

#include <rhi/qshader.h>
#include <QByteArray>
#include <QString>

namespace chaoskit::ui {

// Compiles Vulkan-flavored GLSL source strings at runtime into multi-target
// QShader packs (SPIR-V, MSL, HLSL, and desktop GLSL) for QRhi pipelines.
class RhiShaderBaker {
 public:
  static QShader bake(QShader::Stage stage, const QByteArray& glslSource,
                      QString* errorMessage = nullptr);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_RHISHADERBAKER_H
