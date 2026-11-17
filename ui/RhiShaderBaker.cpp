#include "RhiShaderBaker.h"

#include <rhi/qshaderbaker.h>
#include <QList>

namespace chaoskit::ui {

QShader RhiShaderBaker::bake(QShader::Stage stage, const QByteArray& glslSource,
                             QString* errorMessage) {
  QShaderBaker baker;
  baker.setSourceString(glslSource, stage);
  baker.setGeneratedShaderVariants({QShader::StandardShader});

  QList<QShaderBaker::GeneratedShader> targets;
  if (stage == QShader::ComputeStage) {
    targets = {
        {QShader::SpirvShader, QShaderVersion(100)},
        {QShader::MslShader, QShaderVersion(12)},
        {QShader::HlslShader, QShaderVersion(50)},
        {QShader::GlslShader, QShaderVersion(430)},
    };
  } else {
    targets = {
        {QShader::SpirvShader, QShaderVersion(100)},
        {QShader::MslShader, QShaderVersion(12)},
        {QShader::HlslShader, QShaderVersion(50)},
        {QShader::GlslShader, QShaderVersion(330)},
        {QShader::GlslShader, QShaderVersion(150)},
    };
  }
  baker.setGeneratedShaders(targets);

  QShader shader = baker.bake();
  if (!shader.isValid() && errorMessage != nullptr) {
    *errorMessage = baker.errorMessage();
  }
  return shader;
}

}  // namespace chaoskit::ui
