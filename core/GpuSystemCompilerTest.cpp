#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "CameraSystem.h"
#include "GpuSystemCompiler.h"
#include "Lifetime.h"
#include "TransformIndex.h"
#include "TransformParams.h"
#include "TransformSystem.h"
#include "ast/AffineTransform.h"
#include "ast/Expression.h"
#include "ast/Formula.h"
#include "ast/Input.h"
#include "ast/MultiStepTransform.h"
#include "ast/Output.h"
#include "ast/Parameter.h"
#include "ast/RandomChoiceTransform.h"
#include "ast/Transform.h"
#include "ast/VariableName.h"
#include "ast/WeightedSumTransform.h"
#include "ast/helpers.h"
#include "errors.h"

namespace chaoskit::core {
namespace {

using ast::Input;
using ast::Output;
using ast::Parameter;
using namespace ast::helpers;

struct UniformHeaderView {
  uint32_t width;
  uint32_t height;
  int32_t minLifetime;
  int32_t maxLifetime;
  uint32_t particleCount;
  uint32_t ticksPerInvocation;
  uint32_t seededMode;
  uint32_t reserved0;
  uint32_t seedLo;
  uint32_t seedHi;
  uint32_t tickOffsetLo;
  uint32_t tickOffsetHi;
  float halfMax;
  float halfWidth;
  float halfHeight;
  float reserved1;
};

static_assert(sizeof(UniformHeaderView) == 64);

CameraSystem makeSampleSystem() {
  ast::Transform formulaTransform(ast::Formula{
      sin(Parameter(0) * Input(Input::Y)) - cos(Parameter(1) * Input(Input::X)),
      sin(Parameter(2) * Input(Input::X)) - cos(Parameter(3) * Input(Input::Y)),
  });

  ast::Transform weightedSum(
      ast::WeightedSumTransform({{formulaTransform, 0.75}}));
  ast::Transform blend(ast::MultiStepTransform(
                           {ast::Transform(ast::AffineTransform{}), weightedSum,
                            ast::Transform(ast::AffineTransform{})}),
                       (Input(Input::COLOR) + Parameter(0)) / 2.f);

  TransformParams params;
  params[TransformIndex{0}] = {0.25f};
  params[TransformIndex{0, 0}] = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f};
  params[TransformIndex{0, 1, 0}] = {-1.9f, -1.7f, -1.8f, -1.9f};
  params[TransformIndex{0, 2}] = {0.5f, 0.f, 0.1f, 0.f, 0.5f, -0.1f};

  ast::Transform root(ast::RandomChoiceTransform({{blend, 1.0}}));
  return CameraSystem{TransformSystem{root, params}, std::nullopt,
                      Lifetime(1, 10)};
}

TEST(GpuSystemCompilerTest,
     StructuralKeyIgnoresParamsWeightsAndLifetimeBounds) {
  const CameraSystem base = makeSampleSystem();
  const std::string baseKey = GpuSystemCompiler::computeStructuralKey(base);
  EXPECT_FALSE(baseKey.empty());

  // Mutate TransformParams floats (affine, formula, coloring).
  CameraSystem mutatedParams = base;
  mutatedParams.system.params[TransformIndex{0}] = {0.9f};
  mutatedParams.system.params[TransformIndex{0, 0}] = {2.f,  1.f, -0.5f,
                                                       -1.f, 2.f, 0.25f};
  mutatedParams.system.params[TransformIndex{0, 1, 0}] = {0.1f, 0.2f, 0.3f,
                                                          0.4f};
  EXPECT_EQ(GpuSystemCompiler::computeStructuralKey(mutatedParams), baseKey);

  // Mutate RandomChoiceTransform and WeightedSumTransform weights.
  CameraSystem mutatedWeights = base;
  const auto& rc =
      base.system.transform.variant().get<ast::RandomChoiceTransform>();
  const auto& childBlend = rc.transforms()[0];
  const auto& ms = childBlend.variant().get<ast::MultiStepTransform>();
  const auto& ws =
      ms.transforms()[1].variant().get<ast::WeightedSumTransform>();
  ast::Transform updatedWs(
      ast::WeightedSumTransform({{ws.transforms()[0], 2.5}}));
  ast::Transform updatedBlend(
      ast::MultiStepTransform(
          {ms.transforms()[0], updatedWs, ms.transforms()[2]}),
      childBlend.color());
  mutatedWeights.system.transform =
      ast::Transform(ast::RandomChoiceTransform({{updatedBlend, 0.25}}));
  EXPECT_EQ(GpuSystemCompiler::computeStructuralKey(mutatedWeights), baseKey);

  // Mutate Lifetime bounds (including Infinite).
  CameraSystem mutatedLifetime = base;
  mutatedLifetime.lifetime = Lifetime(5, 100);
  EXPECT_EQ(GpuSystemCompiler::computeStructuralKey(mutatedLifetime), baseKey);

  mutatedLifetime.lifetime = Lifetime(0, Lifetime::INFINITE);
  EXPECT_EQ(GpuSystemCompiler::computeStructuralKey(mutatedLifetime), baseKey);
}

TEST(GpuSystemCompilerTest, StructuralKeyChangesOnStructuralMutations) {
  const CameraSystem base = makeSampleSystem();
  const std::string baseKey = GpuSystemCompiler::computeStructuralKey(base);

  // 1. Add a second blend (RandomChoiceTransform topology change).
  CameraSystem twoBlends = base;
  const auto& rc =
      base.system.transform.variant().get<ast::RandomChoiceTransform>();
  twoBlends.system.transform = ast::Transform(ast::RandomChoiceTransform(
      {{rc.transforms()[0], 1.0}, {rc.transforms()[0], 1.0}}));
  EXPECT_NE(GpuSystemCompiler::computeStructuralKey(twoBlends), baseKey);

  // 2. Toggle Camera presence.
  CameraSystem withCamera = base;
  withCamera.camera = TransformSystem{
      ast::Transform(ast::Formula{Input(Input::X), Input(Input::Y)}), {}};
  const std::string cameraKey =
      GpuSystemCompiler::computeStructuralKey(withCamera);
  EXPECT_NE(cameraKey, baseKey);

  // Mutate camera expression constant -> distinct key.
  CameraSystem mutatedCamera = withCamera;
  mutatedCamera.camera = TransformSystem{
      ast::Transform(ast::Formula{Input(Input::X) + 1.f, Input(Input::Y)}), {}};
  EXPECT_NE(GpuSystemCompiler::computeStructuralKey(mutatedCamera), cameraKey);

  // 3. Change coloring method expression tree.
  CameraSystem diffColor = base;
  const auto& childBlend = rc.transforms()[0];
  diffColor.system.transform = ast::Transform(ast::RandomChoiceTransform(
      {{ast::Transform(childBlend.variant(), Output(Output::COLOR)), 1.0}}));
  EXPECT_NE(GpuSystemCompiler::computeStructuralKey(diffColor), baseKey);

  // 4. Change formula expression tree / literal constant.
  CameraSystem diffFormula = {
      TransformSystem{
          ast::Transform(ast::Formula{Input(Input::X) * 2.f, Input(Input::Y)}),
          {}},
      std::nullopt, Lifetime()};
  CameraSystem diffLiteral = {
      TransformSystem{
          ast::Transform(ast::Formula{Input(Input::X) * 3.f, Input(Input::Y)}),
          {}},
      std::nullopt, Lifetime()};
  EXPECT_NE(GpuSystemCompiler::computeStructuralKey(diffFormula),
            GpuSystemCompiler::computeStructuralKey(diffLiteral));
}

TEST(GpuSystemCompilerTest, PacksHeaderAndParametersIntoStd140Buffer) {
  const CameraSystem sys = makeSampleSystem();
  const CompiledGpuSystem compiled = GpuSystemCompiler::compile(sys);

  EXPECT_EQ(compiled.structuralKey, compiled.layout.structuralKey);
  EXPECT_FALSE(compiled.computeShaderSource.empty());
  EXPECT_GE(compiled.layout.uniformBufferSize, 80u);
  EXPECT_EQ(compiled.layout.uniformBufferSize % 16u, 0u);

  const GpuDispatchConfig config = {
      200, 100, 4096, 32, true, 0x123456789ABCDEF0ULL, 0x0000000100000040ULL,
  };
  const std::vector<uint8_t> bytes =
      GpuSystemCompiler::packUniforms(compiled.layout, sys, config);
  ASSERT_EQ(bytes.size(), compiled.layout.uniformBufferSize);

  UniformHeaderView header = {};
  std::memcpy(&header, bytes.data(), sizeof(header));
  EXPECT_EQ(header.width, 200u);
  EXPECT_EQ(header.height, 100u);
  EXPECT_EQ(header.minLifetime, 1);
  EXPECT_EQ(header.maxLifetime, 10);
  EXPECT_EQ(header.particleCount, 4096u);
  EXPECT_EQ(header.ticksPerInvocation, 32u);
  EXPECT_EQ(header.seededMode, 1u);
  EXPECT_EQ(header.seedLo, 0x9ABCDEF0u);
  EXPECT_EQ(header.seedHi, 0x12345678u);
  EXPECT_EQ(header.tickOffsetLo, 0x00000040u);
  EXPECT_EQ(header.tickOffsetHi, 0x00000001u);
  EXPECT_FLOAT_EQ(header.halfMax, 100.0f);
  EXPECT_FLOAT_EQ(header.halfWidth, 100.0f);
  EXPECT_FLOAT_EQ(header.halfHeight, 50.0f);
}

TEST(GpuSystemCompilerTest, PacksRandomChoiceCumulativeAndZeroWeights) {
  ast::Transform branch(ast::Formula{1.f, 2.f}, 0.5f);
  CameraSystem sys = {
      TransformSystem{
          ast::Transform(ast::RandomChoiceTransform(
              {{branch, 0.0}, {branch, 0.4}, {branch, 0.6}, {branch, 0.0}})),
          {}},
      std::nullopt,
      Lifetime(),
  };

  const CompiledGpuSystem compiled = GpuSystemCompiler::compile(sys);
  ASSERT_EQ(compiled.layout.randomChoiceSlots.size(), 1u);
  const auto slot = compiled.layout.randomChoiceSlots[0];
  EXPECT_EQ(slot.branchCount, 4u);

  const GpuDispatchConfig config = {64, 64, 64, 1, false, 0, 0};
  const std::vector<uint8_t> bytes =
      GpuSystemCompiler::packUniforms(compiled.layout, sys, config);

  float thresholds[4] = {};
  std::memcpy(thresholds,
              bytes.data() + GpuSystemCompiler::kHeaderByteSize +
                  slot.floatOffset * sizeof(float),
              sizeof(thresholds));
  EXPECT_LT(thresholds[0], 0.0f);
  EXPECT_NEAR(thresholds[1], 0.4f, 1e-6f);
  EXPECT_GT(thresholds[2], 1.5f);  // Last positive branch clamped >= 1.0
  EXPECT_LT(thresholds[3], 0.0f);

  // Zero-total-weight RandomChoiceTransform preserves structural key and packs
  // negative thresholds so shader acts as no-op.
  CameraSystem zeroSys = {
      TransformSystem{
          ast::Transform(ast::RandomChoiceTransform(
              {{branch, 0.0}, {branch, -1.0}, {branch, 0.0}, {branch, 0.0}})),
          {}},
      std::nullopt,
      Lifetime(),
  };
  EXPECT_EQ(GpuSystemCompiler::computeStructuralKey(zeroSys),
            compiled.structuralKey);
  const std::vector<uint8_t> zeroBytes =
      GpuSystemCompiler::packUniforms(compiled.layout, zeroSys, config);
  std::memcpy(thresholds,
              zeroBytes.data() + GpuSystemCompiler::kHeaderByteSize +
                  slot.floatOffset * sizeof(float),
              sizeof(thresholds));
  for (float t : thresholds) {
    EXPECT_LT(t, 0.0f);
  }
}

TEST(GpuSystemCompilerTest, RejectsInvalidExpressionsAndMismatchedLayouts) {
  CameraSystem badVar = {
      TransformSystem{ast::Transform(ast::Formula{ast::VariableName("missing"),
                                                  ast::Input(Input::Y)}),
                      {}},
      std::nullopt,
      Lifetime(),
  };
  EXPECT_THROW((void)GpuSystemCompiler::compile(badVar),
               UndefinedVariableError);

  CameraSystem badOutput = {
      TransformSystem{ast::Transform(ast::Formula{ast::Output(ast::Output::X),
                                                  ast::Input(Input::Y)}),
                      {}},
      std::nullopt,
      Lifetime(),
  };
  EXPECT_THROW((void)GpuSystemCompiler::compile(badOutput), OutputNotAvailable);

  const CameraSystem sys = makeSampleSystem();
  const CompiledGpuSystem compiled = GpuSystemCompiler::compile(sys);
  CameraSystem differentSys = {
      TransformSystem{ast::Transform(ast::Formula{0.f, 0.f}), {}},
      std::nullopt,
      Lifetime(),
  };
  EXPECT_THROW(
      (void)GpuSystemCompiler::packUniforms(compiled.layout, differentSys,
                                            GpuDispatchConfig{64, 64, 64, 1}),
      std::invalid_argument);
}

TEST(GpuSystemCompilerTest,
     PacksAffineFormulaCameraAndWeightedSumSlotsAndChecksMissingParams) {
  ast::Transform formula(
      ast::Formula{Parameter(0) * Input(Input::X),
                   Parameter(0) + Parameter(1) * Input(Input::Y)});
  ast::Transform ws(ast::WeightedSumTransform({{formula, 0.35}}));
  ast::Transform pre(ast::AffineTransform{});
  ast::Transform blend(ast::MultiStepTransform({pre, ws}),
                       (Input(Input::COLOR) + Parameter(0)) / 2.f);

  TransformParams sysParams;
  sysParams[TransformIndex{0}] = {0.4f};
  sysParams[TransformIndex{0, 0}] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
  sysParams[TransformIndex{0, 1, 0}] = {7.f, 8.f};

  ast::Transform camFormula(
      ast::Formula{Parameter(0) + Input(Input::X), Input(Input::Y)});
  TransformParams camParams;
  camParams[TransformIndex{}] = {9.f};

  CameraSystem sys = {
      TransformSystem{
          ast::Transform(ast::RandomChoiceTransform({{blend, 1.0}})),
          sysParams},
      TransformSystem{camFormula, camParams},
      Lifetime(),
  };

  const CompiledGpuSystem compiled = GpuSystemCompiler::compile(sys);
  const std::vector<uint8_t> bytes = GpuSystemCompiler::packUniforms(
      compiled.layout, sys, GpuDispatchConfig{64, 64, 64, 1});
  const float* packed = reinterpret_cast<const float*>(
      bytes.data() + GpuSystemCompiler::kHeaderByteSize);

  ASSERT_EQ(compiled.layout.affineSlots.size(), 1u);
  const auto& aff = compiled.layout.affineSlots[0];
  for (int i = 0; i < 6; ++i) {
    EXPECT_FLOAT_EQ(packed[aff.floatOffset + i], static_cast<float>(i + 1));
  }

  ASSERT_EQ(compiled.layout.weightedSumSlots.size(), 1u);
  EXPECT_FLOAT_EQ(packed[compiled.layout.weightedSumSlots[0].floatOffset],
                  0.35f);

  bool sawCamParam = false;
  bool sawFormulaParam1 = false;
  for (const auto& pSlot : compiled.layout.paramSlots) {
    if (pSlot.target.scope == TransformScope::Camera && pSlot.paramIndex == 0) {
      EXPECT_FLOAT_EQ(packed[pSlot.floatOffset], 9.f);
      sawCamParam = true;
    } else if (pSlot.target.scope == TransformScope::System &&
               pSlot.target.index == TransformIndex{0, 1, 0} &&
               pSlot.paramIndex == 1) {
      EXPECT_FLOAT_EQ(packed[pSlot.floatOffset], 8.f);
      sawFormulaParam1 = true;
    }
  }
  EXPECT_TRUE(sawCamParam);
  EXPECT_TRUE(sawFormulaParam1);

  CameraSystem missingParamSys = sys;
  missingParamSys.system.params[TransformIndex{0, 1, 0}] = {7.f};
  EXPECT_THROW(
      (void)GpuSystemCompiler::packUniforms(compiled.layout, missingParamSys,
                                            GpuDispatchConfig{64, 64, 64, 1}),
      MissingParameterError);
}

}  // namespace
}  // namespace chaoskit::core
