#include <gmock/gmock.h>

#include <chaoskit.pb.h>
#include "ast/transform_debug.h"
#include "conversions.h"
#include "core/debug.h"
#include "library/util.h"

using chaoskit::System;
using chaoskit::core::CameraSystem;
using chaoskit::library::ColoringMethodType;
using chaoskit::library::FormulaType;
using namespace testing;

namespace chaoskit::ui {

namespace {

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

  core::CameraSystem actualSystem = toCameraSystem(input);

  ast::Transform expectedTransform{ast::RandomChoiceTransform()};
  core::TransformParams expectedParams;
  EXPECT_EQ(actualSystem,
            (core::CameraSystem{{expectedTransform, expectedParams}}));
}

TEST_F(ConversionsTest, ConvertsBlendsToMultiStep) {
  System input;
  // Two empty but enabled blends with their weights set to 1.
  Blend* first = input.add_blends();
  first->set_enabled(true);
  first->set_weight(1.f);
  Blend* second = input.add_blends();
  second->set_enabled(true);
  second->set_weight(1.f);

  core::CameraSystem actualSystem = toCameraSystem(input);

  std::vector<ast::Transform> expectedTransforms{
      ast::Transform{ast::MultiStepTransform{}},
      ast::Transform{ast::MultiStepTransform{}}};
  std::vector<double> expectedWeights{1, 1};
  ast::Transform expectedTransform{
      ast::RandomChoiceTransform(expectedTransforms, expectedWeights)};
  EXPECT_EQ(actualSystem.system.transform, expectedTransform);
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
}

TEST_F(ConversionsTest, ConvertsPreToAffineTransform) {
  System input;
  // A single blend with just a 2x scale pre transform.
  Blend* blend = input.add_blends();
  blend->set_enabled(true);
  setTransform(blend->mutable_pre(), {2.f, 0.f, 0.f, 0.f, 2.f, 0.f});

  core::CameraSystem actualSystem = toCameraSystem(input);
  const auto& actualRootTransform =
      actualSystem.system.transform.variant().get<ast::RandomChoiceTransform>();

  ast::Transform expectedStep{ast::AffineTransform()};
  ast::Transform expectedTransform{ast::MultiStepTransform{{expectedStep}}};
  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0}] = {2.f, 0.f, 0.f, 0.f, 2.f, 0.f};
  EXPECT_THAT(actualRootTransform.transforms(), ElementsAre(expectedTransform));
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
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

  ast::Transform expectedStep{ast::AffineTransform()};
  ast::Transform expectedTransform{ast::MultiStepTransform{{expectedStep}}};
  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0, 0}] = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f};

  EXPECT_THAT(actualRootTransform.transforms(), ElementsAre(expectedTransform));
  EXPECT_THAT(actualSystem.system.params, ContainerEq(expectedParams));
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
              ElementsAre(ast::Transform{ast::AffineTransform{}}));
}

TEST_F(ConversionsTest, ConvertsFinalBlendToCamera) {
  System input;
  // Just a final blend with a post transform.
  Blend* finalBlend = input.mutable_final_blend();
  finalBlend->set_enabled(true);
  setTransform(finalBlend->mutable_post(), {.5, 0, 0, 0, 1, 0});

  core::CameraSystem actualSystem = toCameraSystem(input);

  ast::Transform expectedStep{ast::AffineTransform()};
  ast::Transform expectedTransform{ast::MultiStepTransform{{expectedStep}}};
  core::TransformParams expectedParams;
  expectedParams[core::TransformIndex{0}] = {.5f, 0.f, 0.f, 0.f, 1.f, 0.f};

  ASSERT_TRUE(actualSystem.camera.has_value());
  EXPECT_THAT(actualSystem.camera->transform, Eq(expectedTransform));
  EXPECT_THAT(actualSystem.camera->params, ContainerEq(expectedParams));
}

TEST_F(ConversionsTest, DoesNotSetCameraIfFinalBlendIsUnset) {
  System input;
  // No final blend.

  core::CameraSystem actualSystem = toCameraSystem(input);

  ASSERT_FALSE(actualSystem.camera.has_value());
}

TEST_F(ConversionsTest, DoesNotSetCameraIfFinalBlendIsDisabled) {
  System input;
  // Disabled final blend.
  input.mutable_final_blend()->set_enabled(false);

  core::CameraSystem actualSystem = toCameraSystem(input);

  ASSERT_FALSE(actualSystem.camera.has_value());
}

}  // namespace chaoskit::ui
