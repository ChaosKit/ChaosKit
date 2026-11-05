#include <gmock/gmock.h>

#include "SystemParticle.h"
#include "SystemProcessor.h"
#include "TransformSystem.h"
#include "testing/MockRng.h"

namespace chaoskit::core {

using namespace ::testing;

class SystemProcessorTest : public testing::Test {
 protected:
  std::shared_ptr<NiceMock<MockRng>> rng =
      std::make_shared<NiceMock<MockRng>>();
};

namespace {

SystemParticle createParticle(float x = 0.f, float y = 0.f, float color = 0.f) {
  return SystemParticle{Point(x, y), color, SystemParticle::INFINITE_LIFETIME,
                        0};
}

}  // namespace

TEST_F(SystemProcessorTest, CreatesParticle) {
  // (output.x, output.y) = (1, 2)
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f})};
  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));

  SystemProcessor processor(system, rng);
  SystemParticle actual = processor.createParticle();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, CreatesParticleWithRandomLifetime) {
  // (output.x, output.y) = (1, 2)
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f})};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(.5f));
  EXPECT_CALL(*rng, randomInt(1, 30)).WillOnce(Return(15));

  SystemProcessor processor(system, rng);
  processor.setMaxLifetime(30);
  SystemParticle actual = processor.createParticle();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.lifetime = 15;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, CreatesParticleWithRandomLifetimeAndWarmUp) {
  // (output.x, output.y) = (1, 2)
  TransformSystem system = {ast::Transform(ast::Formula{1.f, 2.f})};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(.5f));
  EXPECT_CALL(*rng, randomInt(11, 30)).WillOnce(Return(20));

  SystemProcessor processor(system, rng);
  processor.setLifetimeRange(10, 30);
  SystemParticle actual = processor.createParticle();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.lifetime = 20;
  expected.warmup = 10;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, CreatesParticleWhenWarmUpEqualsMaxLifetime) {
  TransformSystem system = {ast::Transform(ast::Formula{1.f, 2.f})};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(.5f));
  EXPECT_CALL(*rng, randomInt(30, 30)).WillOnce(Return(30));

  SystemProcessor processor(system, rng);
  processor.setMinLifetime(30);
  processor.setMaxLifetime(30);
  SystemParticle actual = processor.createParticle();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.lifetime = 30;
  expected.warmup = 30;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, CreatesParticleWithWarmUp) {
  // (output.x, output.y) = (1, 2)
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f})};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(.5f));

  SystemProcessor processor(system, rng);
  processor.setMinLifetime(42);
  SystemParticle actual = processor.createParticle();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.warmup = 42;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, ProcessesSystemWithExistingParticle) {
  // (output.x, output.y, output.color) = (1, 2, 0.25)
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f}, .25f)};
  SystemParticle input = createParticle();

  SystemProcessor processor(system, rng);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(1.f, 2.f, .25f);
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, ProcessesSystemWithNewParticle) {
  // (output.x, output.y) = (input.x, input.y)
  TransformSystem system{ast::Transform(
      ast::Formula{ast::Input(ast::Input::X), ast::Input(ast::Input::Y)})};
  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));

  SystemProcessor processor(system, rng);
  SystemParticle actual = processor.process();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, RevivesDeadParticle) {
  // (output.x, output.y) = (input.x, input.y)
  TransformSystem system{ast::Transform(
      ast::Formula{ast::Input(ast::Input::X), ast::Input(ast::Input::Y)})};
  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));
  SystemParticle input = createParticle();
  input.lifetime = 0;

  SystemProcessor processor(system, rng);
  processor.setMaxLifetime(30);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.lifetime = 29;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, RevivesDeadParticleWithWarmUp) {
  // (output.x, output.y) = (input.x + 1, input.y + 1)
  TransformSystem system = {ast::Transform(
      ast::Formula{ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::X), 1.f),
                   ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::Y), 1.f)})};
  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));
  SystemParticle input = createParticle();
  input.lifetime = 0;

  SystemProcessor processor(system, rng);
  processor.setMaxLifetime(30);
  processor.setMinLifetime(2);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(3.5f, 3.5f, .5f);
  expected.lifetime = 27;
  expected.warmup = 0;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, ExecutesWarmUpSteps) {
  // (output.x, output.y) = (input.x + 1, input.y + 1)
  TransformSystem system{ast::Transform(
      ast::Formula{ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::X), 1.f),
                   ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::Y), 1.f)})};
  SystemParticle input = createParticle();
  input.warmup = 2;

  SystemProcessor processor(system, rng);
  processor.setMinLifetime(2);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(3, 3);
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, WarmUpDoesNotExceedLifetime) {
  // (output.x, output.y) = (input.x + 1, input.y + 1)
  TransformSystem system{ast::Transform(
      ast::Formula{ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::X), 1.f),
                   ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::Y), 1.f)})};
  SystemParticle input = createParticle();
  input.lifetime = 2;
  input.warmup = 4;

  SystemProcessor processor(system, rng);
  processor.setMaxLifetime(2);
  processor.setMinLifetime(4);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(2, 2);
  expected.lifetime = 0;
  expected.warmup = 0;
  ASSERT_EQ(expected, actual);
}

}  // namespace chaoskit::core
