#include <gmock/gmock.h>
#include <array>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include <chaoskit.pb.h>
#include "ast/transform_debug.h"
#include "conversions.h"
#include "core/CameraSystemProcessor.h"
#include "core/Rng.h"
#include "core/SystemParticle.h"
#include "core/debug.h"
#include "library/util.h"

using chaoskit::Blend;
using chaoskit::Formula;
using chaoskit::System;
using chaoskit::Transform;
using chaoskit::core::CameraSystem;
using chaoskit::library::ColoringMethodType;
using chaoskit::library::FormulaType;
using namespace testing;

namespace chaoskit::ui {

namespace {

class MockRng : public core::Rng {
 public:
  MOCK_METHOD(double, randomDouble, (double min, double max), (override));
  MOCK_METHOD(float, randomFloat, (float min, float max), (override));
  MOCK_METHOD(int, randomInt, (int min, int max), (override));
};

void setTransform(Transform* transform, std::array<float, 6> matrix) {
  transform->set_m11(matrix[0]);
  transform->set_m21(matrix[1]);
  transform->set_m31(matrix[2]);
  transform->set_m12(matrix[3]);
  transform->set_m22(matrix[4]);
  transform->set_m32(matrix[5]);
}

void setFormulaParams(Formula* formula, std::initializer_list<float> params) {
  formula->mutable_params()->Add(params.begin(), params.end());
}

MATCHER(IsAnyFormula, "") { return arg.variant().template is<ast::Formula>(); }

MATCHER_P(IsFormula, type, "") {
  return arg.variant().template get<ast::Formula>() == library::source(type);
}

MATCHER_P(HasColoringMethod, type, "") {
  return arg.color() == library::source(type);
}

}  // namespace

class ConversionsTest : public Test {};

TEST_F(ConversionsTest, ConvertsEmptyProtoToEmptyRandomChoice) {
  System input;
  input.set_min_lifetime(0);
  input.set_max_lifetime(core::Lifetime::INFINITE);

  core::CameraSystem actualSystem = toCameraSystem(input);

  auto expectedTransform = ast::Transform(ast::RandomChoiceTransform());
  core::TransformParams expectedParams;
  EXPECT_EQ(actualSystem,
            (core::CameraSystem{{expectedTransform, expectedParams},
                                stdx::nullopt,
                                core::Lifetime(0, core::Lifetime::INFINITE)}));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, core::Lifetime::INFINITE));
}

TEST_F(ConversionsTest, ConvertsBlendsToMultiStep) {
  System input;
  input.set_min_lifetime(10);
  input.set_max_lifetime(30);
  // Two empty but enabled blends with their weights set to 1.
  Blend* first = input.add_blends();
  first->set_enabled(true);
  first->set_weight(1.f);
  Blend* second = input.add_blends();
  second->set_enabled(true);
  second->set_weight(1.f);

  core::CameraSystem actualSystem = toCameraSystem(input);

  std::vector<ast::Transform> expectedTransforms = {
      ast::Transform(ast::MultiStepTransform{}),
      ast::Transform(ast::MultiStepTransform{})};
  std::vector<double> expectedWeights = {1, 1};
  auto expectedTransform = ast::Transform(
      ast::RandomChoiceTransform(expectedTransforms, expectedWeights));
  EXPECT_EQ(actualSystem.system.transform, expectedTransform);
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(10, 30));
}

TEST_F(ConversionsTest, OnlyConvertsEnabledBlends) {
  System input;
  // Two empty blends, only one of which is enabled.
  input.add_blends()->set_enabled(true);
  input.add_blends()->set_enabled(false);

  core::CameraSystem actualSystem = toCameraSystem(input);
  const auto& actualRootTransform =
      actualSystem.system.transform.variant().get<ast::RandomChoiceTransform>();

  ASSERT_THAT(actualRootTransform.transforms(), SizeIs(1));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, ConvertsPreToAffineTransform) {
  System input;
  input.set_min_lifetime(5);
  input.set_max_lifetime(50);
  // A single blend with just a 2x scale pre transform.
  Blend* blend = input.add_blends();
  blend->set_enabled(true);
  setTransform(blend->mutable_pre(), {1.f, 2.f, 3.f, 4.f, 5.f, 6.f});

  core::CameraSystem actualSystem = toCameraSystem(input);
  const auto& actualRootTransform =
      actualSystem.system.transform.variant().get<ast::RandomChoiceTransform>();

  auto expectedStep = ast::Transform(ast::AffineTransform());
  auto expectedTransform =
      ast::Transform(ast::MultiStepTransform{{expectedStep}});
  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0}] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
  EXPECT_THAT(actualRootTransform.transforms(), ElementsAre(expectedTransform));
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(5, 50));
}

TEST_F(ConversionsTest, ConvertsFormulasToWeightedSum) {
  System input;
  // A single blend with a DeJong formula with a 2.0 weight and some params.
  Blend* blend = input.add_blends();
  blend->set_enabled(true);
  Formula* formula = blend->add_formulas();
  formula->set_type("DeJong");
  formula->mutable_weight()->set_x(2.f);
  setFormulaParams(formula, {1.f, 2.f, 3.f, 4.f});

  core::CameraSystem actualSystem = toCameraSystem(input);
  const auto& actualRootTransform =
      actualSystem.system.transform.variant().get<ast::RandomChoiceTransform>();
  // There's only one blend.
  ASSERT_THAT(actualRootTransform.transforms(), SizeIs(1));
  const auto& actualMultiStep = actualRootTransform.transforms()
                                    .front()
                                    .variant()
                                    .get<ast::MultiStepTransform>();
  // Just the formula is defined, so that's the only step.
  ASSERT_THAT(actualMultiStep.transforms(), SizeIs(1));
  const ast::TransformVariant& actualTransformVariant =
      actualMultiStep.transforms().front().variant();
  ASSERT_TRUE(actualTransformVariant.is<ast::WeightedSumTransform>());
  const auto& actualSum =
      actualTransformVariant.get_unchecked<ast::WeightedSumTransform>();

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0, 0}] = {1.f, 2.f, 3.f, 4.f};
  EXPECT_THAT(actualSum.transforms(),
              ElementsAre(IsFormula(FormulaType::DeJong)));
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, ConvertsPostToAffineTransform) {
  System input;
  // A single blend with just an identity post transform.
  Blend* blend = input.add_blends();
  blend->set_enabled(true);
  setTransform(blend->mutable_post(), {1.f, 0.f, 0.f, 0.f, 1.f, 0.f});

  core::CameraSystem actualSystem = toCameraSystem(input);
  const auto& actualRootTransform =
      actualSystem.system.transform.variant().get<ast::RandomChoiceTransform>();

  auto expectedStep = ast::Transform(ast::AffineTransform());
  auto expectedTransform =
      ast::Transform(ast::MultiStepTransform{{expectedStep}});
  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0}] = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f};

  EXPECT_THAT(actualRootTransform.transforms(), ElementsAre(expectedTransform));
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, ConvertsSingleColorColoringMethod) {
  System input;
  // A single blend with a single-color coloring method.
  Blend* blend = input.add_blends();
  blend->set_enabled(true);
  blend->mutable_coloring_method()->set_single_color(.5f);

  core::CameraSystem actualSystem = toCameraSystem(input);
  const auto& actualRootTransform =
      actualSystem.system.transform.variant().get<ast::RandomChoiceTransform>();

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0}] = {.5f};
  EXPECT_THAT(actualRootTransform.transforms(),
              ElementsAre(HasColoringMethod(ColoringMethodType::SingleColor)));
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, ConvertsDistanceColoringMethod) {
  System input;
  // A single blend with a distance-based coloring method.
  Blend* blend = input.add_blends();
  blend->set_enabled(true);
  blend->mutable_coloring_method()->set_distance(.25f);

  core::CameraSystem actualSystem = toCameraSystem(input);
  const auto& actualRootTransform =
      actualSystem.system.transform.variant().get<ast::RandomChoiceTransform>();

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0}] = {.25f};
  EXPECT_THAT(actualRootTransform.transforms(),
              ElementsAre(HasColoringMethod(ColoringMethodType::Distance)));
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, ConvertsOnlyIsolatedBlendIfSet) {
  System input;
  // Two blends with different contents. The second one is set as the isolated
  // blend.
  input.add_blends()->set_enabled(true);
  Blend* secondBlend = input.add_blends();
  secondBlend->set_enabled(true);
  setTransform(secondBlend->mutable_pre(), {1.f, 0.f, 0.f, 0.f, 1.f, 0.f});
  input.mutable_isolated_blend()->set_index(1);

  core::CameraSystem actualSystem = toCameraSystem(input);
  ASSERT_TRUE(
      actualSystem.system.transform.variant().is<ast::MultiStepTransform>());
  const auto& actualMultiStep = actualSystem.system.transform.variant()
                                    .get_unchecked<ast::MultiStepTransform>();

  EXPECT_THAT(actualMultiStep.transforms(),
              ElementsAre(ast::Transform(ast::AffineTransform())));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, ConvertsCamera) {
  System input;
  // Just a camera blend with a post transform.
  Blend* camera = input.mutable_camera();
  camera->set_enabled(true);
  setTransform(camera->mutable_post(), {.5, 0, 0, 0, 1, 0});

  core::CameraSystem actualSystem = toCameraSystem(input);

  auto expectedStep = ast::Transform(ast::AffineTransform());
  auto expectedTransform =
      ast::Transform(ast::MultiStepTransform{{expectedStep}});
  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0}] = {.5f, 0.f, 0.f, 0.f, 1.f, 0.f};

  ASSERT_TRUE(actualSystem.camera.has_value());
  EXPECT_THAT(actualSystem.camera->transform, Eq(expectedTransform));
  EXPECT_THAT(actualSystem.camera->params, ContainerEq(expectedParams));
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, DoesNotSetCameraIfCameraIsUnset) {
  System input;
  // No camera.

  core::CameraSystem actualSystem = toCameraSystem(input);

  ASSERT_FALSE(actualSystem.camera.has_value());
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, DoesNotSetCameraIfCameraIsDisabled) {
  System input;
  // Disabled camera.
  input.mutable_camera()->set_enabled(false);

  core::CameraSystem actualSystem = toCameraSystem(input);

  ASSERT_FALSE(actualSystem.camera.has_value());
  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
}

TEST_F(ConversionsTest, ConvertsSingleBlendToTransformSystem) {
  Blend blend;
  blend.set_enabled(true);
  setTransform(blend.mutable_pre(), {1.f, 2.f, 3.f, 4.f, 5.f, 6.f});

  Formula* formula = blend.add_formulas();
  formula->set_type("DeJong");
  formula->mutable_weight()->set_x(2.f);
  setFormulaParams(formula, {1.f, 2.f, 3.f, 4.f});

  setTransform(blend.mutable_post(), {7.f, 8.f, 9.f, 10.f, 11.f, 12.f});
  blend.mutable_coloring_method()->set_single_color(0.5f);

  std::vector<std::string> warnings;
  core::TransformSystem actual = toTransformSystem(blend, &warnings);

  EXPECT_THAT(warnings, IsEmpty());

  ASSERT_TRUE(actual.transform.variant().is<ast::MultiStepTransform>());
  const auto& actualMultiStep =
      actual.transform.variant().get_unchecked<ast::MultiStepTransform>();
  ASSERT_THAT(actualMultiStep.transforms(), SizeIs(3));

  EXPECT_TRUE(
      actualMultiStep.transforms()[0].variant().is<ast::AffineTransform>());
  EXPECT_TRUE(actualMultiStep.transforms()[1]
                  .variant()
                  .is<ast::WeightedSumTransform>());
  EXPECT_TRUE(
      actualMultiStep.transforms()[2].variant().is<ast::AffineTransform>());
  EXPECT_THAT(actual.transform,
              HasColoringMethod(ColoringMethodType::SingleColor));

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0}] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
  expectedParams[core::TransformIndex{1, 0}] = {1.f, 2.f, 3.f, 4.f};
  expectedParams[core::TransformIndex{2}] = {7.f, 8.f, 9.f, 10.f, 11.f, 12.f};
  expectedParams[core::TransformIndex{}] = {0.5f};

  EXPECT_THAT(actual.params, ContainerEq(expectedParams));
}

TEST_F(ConversionsTest, SkipsUnknownFormulaAndRecordsWarning) {
  Blend blend;
  blend.set_enabled(true);

  Formula* invalidFormula = blend.add_formulas();
  invalidFormula->set_type("NonExistentFormulaType");
  invalidFormula->mutable_weight()->set_x(1.f);
  setFormulaParams(invalidFormula, {99.f});

  Formula* validFormula = blend.add_formulas();
  validFormula->set_type("DeJong");
  validFormula->mutable_weight()->set_x(1.5f);
  setFormulaParams(validFormula, {1.f, 2.f, 3.f, 4.f});

  std::vector<std::string> warnings;
  core::TransformSystem actual = toTransformSystem(blend, &warnings);

  EXPECT_THAT(warnings,
              ElementsAre("Unrecognized formula type: NonExistentFormulaType"));

  ASSERT_TRUE(actual.transform.variant().is<ast::MultiStepTransform>());
  const auto& actualMultiStep =
      actual.transform.variant().get_unchecked<ast::MultiStepTransform>();
  ASSERT_THAT(actualMultiStep.transforms(), SizeIs(1));

  const auto& sumTransform = actualMultiStep.transforms()[0]
                                 .variant()
                                 .get<ast::WeightedSumTransform>();
  EXPECT_THAT(sumTransform.transforms(),
              ElementsAre(IsFormula(FormulaType::DeJong)));
  EXPECT_THAT(sumTransform.weights(), ElementsAre(1.5));

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0}] = {1.f, 2.f, 3.f, 4.f};
  EXPECT_THAT(actual.params, ContainerEq(expectedParams));
}

TEST_F(ConversionsTest,
       SkipsInvalidFormulaEnumAndRecordsWarningWithoutThrowing) {
  Blend blend;
  blend.set_enabled(true);

  Formula* invalidFormula = blend.add_formulas();
  invalidFormula->set_type("Invalid");
  invalidFormula->mutable_weight()->set_x(1.f);
  setFormulaParams(invalidFormula, {99.f});

  Formula* validFormula = blend.add_formulas();
  validFormula->set_type("DeJong");
  validFormula->mutable_weight()->set_x(1.5f);
  setFormulaParams(validFormula, {1.f, 2.f, 3.f, 4.f});

  std::vector<std::string> warnings;
  stdx::optional<core::TransformSystem> actual;
  EXPECT_NO_THROW({ actual = toTransformSystem(blend, &warnings); });
  ASSERT_TRUE(actual.has_value());

  EXPECT_THAT(warnings, ElementsAre("Unrecognized formula type: Invalid"));

  ASSERT_TRUE(actual->transform.variant().is<ast::MultiStepTransform>());
  const auto& actualMultiStep =
      actual->transform.variant().get_unchecked<ast::MultiStepTransform>();
  ASSERT_THAT(actualMultiStep.transforms(), SizeIs(1));

  const auto& sumTransform = actualMultiStep.transforms()[0]
                                 .variant()
                                 .get<ast::WeightedSumTransform>();
  EXPECT_THAT(sumTransform.transforms(),
              ElementsAre(IsFormula(FormulaType::DeJong)));
  EXPECT_THAT(sumTransform.weights(), ElementsAre(1.5));

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0}] = {1.f, 2.f, 3.f, 4.f};
  EXPECT_THAT(actual->params, ContainerEq(expectedParams));
}

TEST_F(ConversionsTest, OmitsWeightedSumWhenAllFormulasAreUnknown) {
  Blend blend;
  blend.set_enabled(true);
  setTransform(blend.mutable_pre(), {1.f, 2.f, 3.f, 4.f, 5.f, 6.f});

  Formula* formula1 = blend.add_formulas();
  formula1->set_type("Unknown1");
  Formula* formula2 = blend.add_formulas();
  formula2->set_type("Unknown2");

  setTransform(blend.mutable_post(), {7.f, 8.f, 9.f, 10.f, 11.f, 12.f});
  blend.mutable_coloring_method()->set_single_color(0.25f);

  std::vector<std::string> warnings;
  core::TransformSystem actual = toTransformSystem(blend, &warnings);

  EXPECT_THAT(warnings, ElementsAre("Unrecognized formula type: Unknown1",
                                    "Unrecognized formula type: Unknown2"));

  ASSERT_TRUE(actual.transform.variant().is<ast::MultiStepTransform>());
  const auto& actualMultiStep =
      actual.transform.variant().get_unchecked<ast::MultiStepTransform>();
  // Only pre and post; WeightedSumTransform was omitted!
  ASSERT_THAT(actualMultiStep.transforms(), SizeIs(2));
  EXPECT_TRUE(
      actualMultiStep.transforms()[0].variant().is<ast::AffineTransform>());
  EXPECT_TRUE(
      actualMultiStep.transforms()[1].variant().is<ast::AffineTransform>());

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0}] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
  expectedParams[core::TransformIndex{1}] = {7.f, 8.f, 9.f, 10.f, 11.f, 12.f};
  expectedParams[core::TransformIndex{}] = {0.25f};
  EXPECT_THAT(actual.params, ContainerEq(expectedParams));
}

TEST_F(ConversionsTest, FallsBackToEnabledBlendsWhenIsolatedIndexIsOutOfRange) {
  System input;
  input.set_min_lifetime(0);
  input.set_max_lifetime(core::Lifetime::INFINITE);

  Blend* first = input.add_blends();
  first->set_enabled(true);
  first->set_weight(1.f);
  setTransform(first->mutable_pre(), {1.f, 0.f, 0.f, 0.f, 1.f, 0.f});

  Blend* second = input.add_blends();
  second->set_enabled(false);

  input.mutable_isolated_blend()->set_index(99);

  std::vector<std::string> warnings;
  core::CameraSystem actualSystem = toCameraSystem(input, &warnings);

  EXPECT_THAT(warnings, ElementsAre("Isolated blend index 99 is out of range"));

  ASSERT_TRUE(
      actualSystem.system.transform.variant().is<ast::RandomChoiceTransform>());
  const auto& randomChoice = actualSystem.system.transform.variant()
                                 .get_unchecked<ast::RandomChoiceTransform>();
  ASSERT_THAT(randomChoice.transforms(), SizeIs(1));

  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0}] = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f};
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
}

TEST_F(ConversionsTest, NormalizesInvalidLifetimeAndRecordsWarning) {
  System input;
  input.set_min_lifetime(-10);
  input.set_max_lifetime(0);

  std::vector<std::string> warnings;
  core::CameraSystem actualSystem = toCameraSystem(input, &warnings);

  EXPECT_EQ(actualSystem.lifetime, core::Lifetime(0, 1));
  EXPECT_THAT(
      warnings,
      ElementsAre("Particle lifetime [-10, 0] was normalized to [0, 1]"));

  // Valid lifetime records no warning
  System validInput;
  validInput.set_min_lifetime(5);
  validInput.set_max_lifetime(20);

  std::vector<std::string> noWarnings;
  core::CameraSystem validSystem = toCameraSystem(validInput, &noWarnings);
  EXPECT_EQ(validSystem.lifetime, core::Lifetime(5, 20));
  EXPECT_THAT(noWarnings, IsEmpty());

  // Infinite lifetime normalization formats "Infinite"
  System infiniteInput;
  infiniteInput.set_min_lifetime(-5);
  infiniteInput.set_max_lifetime(core::Lifetime::INFINITE);

  std::vector<std::string> infiniteWarnings;
  core::CameraSystem infiniteSystem =
      toCameraSystem(infiniteInput, &infiniteWarnings);
  EXPECT_EQ(infiniteSystem.lifetime,
            core::Lifetime(0, core::Lifetime::INFINITE));
  EXPECT_THAT(infiniteWarnings,
              ElementsAre("Particle lifetime [-5, Infinite] was normalized to "
                          "[0, Infinite]"));
}

TEST_F(ConversionsTest, BehavioralRoundTripThroughCameraSystemProcessor) {
  System input;
  input.set_min_lifetime(0);
  input.set_max_lifetime(core::Lifetime::INFINITE);

  Blend* blend = input.add_blends();
  blend->set_enabled(true);
  blend->set_weight(1.f);

  // Pre: scale x by 2, shift x by 1, y identity: x' = 2x + 1, y' = y
  // Affine transform params: [m11, m21, m31, m12, m22, m32]
  setTransform(blend->mutable_pre(), {2.f, 0.f, 1.f, 0.f, 1.f, 0.f});

  // Formula: DeJong with params [0, 0, 0, 0]
  // sin(0*y) - cos(0*x) = -1
  // sin(0*x) - cos(0*y) = -1
  // Output point is (-1, -1) with weight 1.0
  Formula* formula = blend->add_formulas();
  formula->set_type("DeJong");
  formula->mutable_weight()->set_x(1.f);
  setFormulaParams(formula, {0.f, 0.f, 0.f, 0.f});

  // Post: shift x by 3, shift y by 4: x'' = x + 3, y'' = y + 4
  // For (-1, -1) -> (-1 + 3, -1 + 4) = (2, 3)
  setTransform(blend->mutable_post(), {1.f, 0.f, 3.f, 0.f, 1.f, 4.f});

  // Coloring method: SingleColor = 0.8f
  blend->mutable_coloring_method()->set_single_color(0.8f);

  // Camera: scale by 2
  // For (2, 3) -> (4, 6)
  Blend* camera = input.mutable_camera();
  camera->set_enabled(true);
  setTransform(camera->mutable_post(), {2.f, 0.f, 0.f, 0.f, 2.f, 0.f});

  std::vector<std::string> warnings;
  core::CameraSystem cameraSystem = toCameraSystem(input, &warnings);
  EXPECT_THAT(warnings, IsEmpty());

  auto rng = std::make_shared<NiceMock<MockRng>>();
  ON_CALL(*rng, randomDouble(_, _)).WillByDefault(Return(0.5));

  core::CameraSystemProcessor processor(cameraSystem, rng);

  auto particle = core::SystemParticle{core::Point(5.f, 5.f), 0.1f, 100, 0};
  core::SystemParticle stepped = processor.process(particle);

  // Blend transforms:
  // pre: (5, 5) -> (2*5 + 1, 5) = (11, 5)
  // formula: DeJong(0, 0, 0, 0) with weight 1.0 -> (-1, -1)
  // post: (-1, -1) -> (-1 + 3, -1 + 4) = (2, 3)
  // coloring method: SingleColor computes (input.color + param) / 2 = (0.1 +
  // 0.8) / 2 = 0.45
  EXPECT_FLOAT_EQ(stepped.x(), 2.f);
  EXPECT_FLOAT_EQ(stepped.y(), 3.f);
  EXPECT_FLOAT_EQ(stepped.color, 0.45f);

  // Camera transform:
  // post: (2, 3) -> (2*2, 2*3) = (4, 6)
  // camera does not change color
  core::SystemParticle plotted = processor.processCamera(stepped);
  EXPECT_FLOAT_EQ(plotted.x(), 4.f);
  EXPECT_FLOAT_EQ(plotted.y(), 6.f);
  EXPECT_FLOAT_EQ(plotted.color, 0.45f);
}

}  // namespace chaoskit::ui
