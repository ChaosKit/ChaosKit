#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rhi/qshader.h>
#include <QByteArray>
#include <QString>
#include <magic_enum.hpp>

#include <cstdint>
#include <optional>
#include <vector>

#include "RhiComputeEngine.h"
#include "RhiShaderBaker.h"
#include "RhiToneMapper.h"
#include "ast/AffineTransform.h"
#include "ast/Expression.h"
#include "ast/Formula.h"
#include "ast/Input.h"
#include "ast/MultiStepTransform.h"
#include "ast/RandomChoiceTransform.h"
#include "ast/Transform.h"
#include "ast/WeightedSumTransform.h"
#include "ast/helpers.h"
#include "core/CameraSystem.h"
#include "core/GpuSystemCompiler.h"
#include "core/Lifetime.h"
#include "core/TransformIndex.h"
#include "core/TransformParams.h"
#include "core/TransformSystem.h"
#include "library/ColoringMethodType.h"
#include "library/FormulaType.h"
#include "library/util.h"

namespace chaoskit::ui {
namespace {

core::CameraSystem buildBlendSystem(library::FormulaType formulaType,
                                    library::ColoringMethodType coloringType,
                                    bool withCamera) {
  const ast::Formula formulaAst = library::source(formulaType);
  const uint32_t formulaParamCount = library::paramCount(formulaType);
  std::vector<float> formulaParams = library::exampleParams(formulaType);
  if (formulaParams.size() < formulaParamCount) {
    formulaParams.resize(formulaParamCount, 1.0f);
  }

  const ast::Expression colorExpr = library::source(coloringType);
  const uint32_t colorParamCount = library::paramCount(coloringType);

  ast::Transform weightedSum(
      ast::WeightedSumTransform({{ast::Transform(formulaAst), 1.0}}));
  ast::Transform blend(ast::MultiStepTransform(
                           {ast::Transform(ast::AffineTransform{}), weightedSum,
                            ast::Transform(ast::AffineTransform{})}),
                       colorExpr);

  core::TransformParams params;
  params[core::TransformIndex{0}] = std::vector<float>(colorParamCount, 0.5f);
  params[core::TransformIndex{0, 0}] = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f};
  params[core::TransformIndex{0, 1, 0}] = formulaParams;
  params[core::TransformIndex{0, 2}] = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f};

  ast::Transform root(ast::RandomChoiceTransform({{blend, 1.0}}));
  std::optional<core::TransformSystem> camera = std::nullopt;
  if (withCamera) {
    camera = core::TransformSystem{
        ast::Transform(
            ast::MultiStepTransform({ast::Transform(ast::AffineTransform{}),
                                     ast::Transform(ast::WeightedSumTransform(
                                         {{ast::Transform(library::source(
                                               library::FormulaType::Linear)),
                                           1.0}}))})),
        {
            {core::TransformIndex{0}, {1.f, 0.f, 0.f, 0.f, 1.f, 0.f}},
        }};
  }
  return core::CameraSystem{core::TransformSystem{root, params}, camera,
                            core::Lifetime(1, 10)};
}

TEST(ShaderBakerTest, CompilesAndBakesEveryFormulaAndColoringMethod) {
  for (library::FormulaType formulaType :
       magic_enum::enum_values<library::FormulaType>()) {
    if (formulaType == library::FormulaType::Invalid) {
      continue;
    }
    for (library::ColoringMethodType coloringType :
         magic_enum::enum_values<library::ColoringMethodType>()) {
      for (bool withCamera : {false, true}) {
        const core::CameraSystem system =
            buildBlendSystem(formulaType, coloringType, withCamera);
        const core::CompiledGpuSystem compiled =
            core::GpuSystemCompiler::compile(system);

        QString error;
        const QShader shader = RhiShaderBaker::bake(
            QShader::ComputeStage,
            QByteArray::fromStdString(compiled.computeShaderSource), &error);
        EXPECT_TRUE(shader.isValid())
            << "Failed baking formula="
            << std::string(magic_enum::enum_name(formulaType))
            << " coloring=" << std::string(magic_enum::enum_name(coloringType))
            << " withCamera=" << withCamera << " error=" << error.toStdString();
        EXPECT_TRUE(error.isEmpty()) << error.toStdString();
      }
    }
  }
}

TEST(ShaderBakerTest, CompilesAndBakesAllUnaryAndBinaryAstOperators) {
  using namespace ast::helpers;
  const ast::Input ix(ast::Input::X);
  const ast::Input iy(ast::Input::Y);

  const ast::Expression allUnary = sin(ix) + cos(iy) + tan(ix) + (-ix) +
                                   sqrt(abs(ix)) + atan(iy) + trunc(ix) +
                                   exp(ix) + floor(iy) + ceil(ix) + signum(iy) +
                                   (!ix) + frac(iy) + sinh(ix) + cosh(iy);

  const ast::Expression allBinary = ((ix + iy) - (ix * iy)) / (abs(iy) + 1.f) +
                                    pow(ix, 3.f) + (ix % (abs(iy) + 1.f)) +
                                    (gt(ix, 0.f) && lt(iy, 0.f)) +
                                    (eq(ix, iy) || lte(ix, iy)) + gte(ix, iy) +
                                    distance(ix, iy) + atan2(iy, ix) + random();

  const core::CameraSystem sys = {
      core::TransformSystem{ast::Transform(ast::Formula{allUnary, allBinary}),
                            {}},
      std::nullopt,
      core::Lifetime(),
  };

  const core::CompiledGpuSystem compiled =
      core::GpuSystemCompiler::compile(sys);
  QString error;
  const QShader shader = RhiShaderBaker::bake(
      QShader::ComputeStage,
      QByteArray::fromStdString(compiled.computeShaderSource), &error);
  EXPECT_TRUE(shader.isValid()) << error.toStdString();
  EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

TEST(ShaderBakerTest, BakesBuiltinComputeAndSsboToneMapperShaders) {
  QString clearErr;
  const QShader clearShader = RhiShaderBaker::bake(
      QShader::ComputeStage, RhiComputeEngine::clearShaderSource(), &clearErr);
  EXPECT_TRUE(clearShader.isValid()) << clearErr.toStdString();
  EXPECT_TRUE(clearErr.isEmpty()) << clearErr.toStdString();

  QString initErr;
  const QShader initShader = RhiShaderBaker::bake(
      QShader::ComputeStage, RhiComputeEngine::initShaderSource(), &initErr);
  EXPECT_TRUE(initShader.isValid()) << initErr.toStdString();
  EXPECT_TRUE(initErr.isEmpty()) << initErr.toStdString();

  QString vsErr;
  const QShader ssboVs = RhiToneMapper::bakeSsboVertexShader(&vsErr);
  EXPECT_TRUE(ssboVs.isValid()) << vsErr.toStdString();
  EXPECT_TRUE(vsErr.isEmpty()) << vsErr.toStdString();

  QString fsErr;
  const QShader ssboFs = RhiToneMapper::bakeSsboFragmentShader(&fsErr);
  EXPECT_TRUE(ssboFs.isValid()) << fsErr.toStdString();
  EXPECT_TRUE(fsErr.isEmpty()) << fsErr.toStdString();
}

}  // namespace
}  // namespace chaoskit::ui
