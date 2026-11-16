#include <gmock/gmock.h>
#include <cmath>

#include "SystemParticle.h"
#include "SystemProcessor.h"
#include "TransformSystem.h"
#include "ast/helpers.h"
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

TEST_F(SystemProcessorTest,
       MarksFiniteLifetimeParticleExpiredOnNonFiniteStepAndRevivesOnNextStep) {
  using namespace ast::helpers;

  // x' = 1 / x (produces +Inf when x == 0), y' = 0, color' = input.color
  TransformSystem system = {
      ast::Transform(ast::Formula{1.f / ast::Input(ast::Input::X), 0.f},
                     ast::Input(ast::Input::COLOR)),
      {}};
  SystemParticle input(Point(0.f, 0.f), .25f, 5, 0);

  EXPECT_CALL(*rng, randomFloat(_, _)).Times(0);

  SystemProcessor processor(system, rng);
  processor.setMaxLifetime(5);
  SystemParticle actual = processor.process(input);

  EXPECT_EQ(actual.lifetime, 0);
  EXPECT_EQ(actual.warmup, 0);
  EXPECT_FALSE(std::isfinite(actual.x()));

  testing::Mock::VerifyAndClearExpectations(rng.get());
  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));

  SystemParticle revived = processor.process(actual);
  EXPECT_TRUE(std::isfinite(revived.x()));
  EXPECT_TRUE(std::isfinite(revived.y()));
  EXPECT_TRUE(std::isfinite(revived.color));
  EXPECT_FLOAT_EQ(revived.x(), 2.f);
  EXPECT_FLOAT_EQ(revived.y(), 0.f);
  EXPECT_FLOAT_EQ(revived.color, .5f);
  EXPECT_EQ(revived.lifetime, 4);
  EXPECT_EQ(revived.warmup, 0);
}

TEST_F(
    SystemProcessorTest,
    MarksInfiniteLifetimeParticleExpiredOnNonFiniteStepAndRevivesOnNextStep) {
  using namespace ast::helpers;

  // y' = sqrt(y) (produces NaN when y < 0)
  TransformSystem system = {
      ast::Transform(ast::Formula{0.f, sqrt(ast::Input(ast::Input::Y))},
                     ast::Input(ast::Input::COLOR)),
      {}};
  SystemParticle input(Point(0.f, -1.f), .25f,
                       SystemParticle::INFINITE_LIFETIME, 0);

  SystemProcessor processor(system, rng);
  SystemParticle actual = processor.process(input);

  EXPECT_EQ(actual.lifetime, 0);
  EXPECT_EQ(actual.warmup, 0);
  EXPECT_FALSE(std::isfinite(actual.y()));

  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.25f));

  SystemParticle revived = processor.process(actual);
  EXPECT_TRUE(std::isfinite(revived.x()));
  EXPECT_TRUE(std::isfinite(revived.y()));
  EXPECT_TRUE(std::isfinite(revived.color));
  EXPECT_FLOAT_EQ(revived.y(), .5f);
  EXPECT_EQ(revived.lifetime, SystemParticle::INFINITE_LIFETIME);
}

TEST_F(SystemProcessorTest,
       MarksParticleExpiredWhenColorCoordinateBecomesNonFinite) {
  using namespace ast::helpers;

  TransformSystem system = {ast::Transform(ast::Formula{0.f, 0.f},
                                           1.f / ast::Input(ast::Input::COLOR)),
                            {}};
  SystemParticle input(Point(0.f, 0.f), 0.f, SystemParticle::INFINITE_LIFETIME,
                       0);

  SystemProcessor processor(system, rng);
  SystemParticle actual = processor.process(input);

  EXPECT_EQ(actual.lifetime, 0);
  EXPECT_EQ(actual.warmup, 0);
  EXPECT_FALSE(std::isfinite(actual.color));

  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));
  SystemParticle revived = processor.process(actual);
  EXPECT_FLOAT_EQ(revived.color, 2.f);
  EXPECT_EQ(revived.lifetime, SystemParticle::INFINITE_LIFETIME);
}

TEST_F(SystemProcessorTest,
       NonFiniteWarmupStepExpiresImmediatelyAndRevivesOnNextStep) {
  using namespace ast::helpers;

  TransformSystem system = {
      ast::Transform(ast::Formula{1.f / ast::Input(ast::Input::X), 0.f},
                     ast::Input(ast::Input::COLOR)),
      {}};
  SystemParticle input(Point(0.f, 0.f), .25f, 10, 3);

  EXPECT_CALL(*rng, randomFloat(_, _)).Times(0);

  SystemProcessor processor(system, rng);
  processor.setLifetimeRange(3, 10);
  SystemParticle actual = processor.process(input);

  EXPECT_EQ(actual.lifetime, 0);
  EXPECT_EQ(actual.warmup, 0);
  EXPECT_FALSE(std::isfinite(actual.x()));

  testing::Mock::VerifyAndClearExpectations(rng.get());
  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));

  // Revives to max_lifetime=10, runs 3 warmup steps + 1 plotted step => 6.
  SystemParticle revived = processor.process(actual);
  EXPECT_TRUE(std::isfinite(revived.x()));
  EXPECT_EQ(revived.lifetime, 6);
  EXPECT_EQ(revived.warmup, 0);
}

TEST_F(SystemProcessorTest, SingleStepExecutesWarmUpAndRevivesCorrectly) {
  using namespace ast::helpers;

  // x' = x + 1, y' = y + 1
  TransformSystem system = {
      ast::Transform(ast::Formula{ast::Input(ast::Input::X) + 1.f,
                                  ast::Input(ast::Input::Y) + 1.f},
                     ast::Input(ast::Input::COLOR)),
      {}};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(0.f));
  EXPECT_CALL(*rng, randomInt(3, 5)).WillOnce(Return(4));

  SystemProcessor processor(system, rng);
  processor.setLifetimeRange(2, 5);

  SystemParticle particle = processor.createParticle();
  EXPECT_EQ(particle.lifetime, 4);
  EXPECT_EQ(particle.warmup, 2);

  // Warmup tick 1 & 2 return false.
  EXPECT_FALSE(processor.step(particle));
  EXPECT_EQ(particle.lifetime, 3);
  EXPECT_EQ(particle.warmup, 1);

  EXPECT_FALSE(processor.step(particle));
  EXPECT_EQ(particle.lifetime, 2);
  EXPECT_EQ(particle.warmup, 0);

  // Plotted ticks 3 & 4 return true.
  EXPECT_TRUE(processor.step(particle));
  EXPECT_FLOAT_EQ(particle.x(), 3.f);
  EXPECT_EQ(particle.lifetime, 1);

  EXPECT_TRUE(processor.step(particle));
  EXPECT_FLOAT_EQ(particle.x(), 4.f);
  EXPECT_EQ(particle.lifetime, 0);

  // Tick 5 revives with max_lifetime=5, warmup=2 -> executes 1st warmup tick.
  EXPECT_FALSE(processor.step(particle));
  EXPECT_FLOAT_EQ(particle.x(), 1.f);
  EXPECT_EQ(particle.lifetime, 4);
  EXPECT_EQ(particle.warmup, 1);
}

TEST_F(SystemProcessorTest, SingleStepNeverPlotsWhenWarmUpEqualsMaxLifetime) {
  using namespace ast::helpers;

  TransformSystem system = {
      ast::Transform(ast::Formula{ast::Input(ast::Input::X) + 1.f, 0.f}), {}};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(0.f));
  EXPECT_CALL(*rng, randomInt(2, 2)).WillOnce(Return(2));

  SystemProcessor processor(system, rng);
  processor.setLifetimeRange(2, 2);

  SystemParticle particle = processor.createParticle();
  for (int i = 0; i < 6; ++i) {
    EXPECT_FALSE(processor.step(particle));
  }
}

TEST_F(SystemProcessorTest,
       SingleStepNonFiniteExpiresUnderInfiniteLifetimeAndRevivesNextTick) {
  using namespace ast::helpers;

  TransformSystem system = {
      ast::Transform(ast::Formula{1.f / ast::Input(ast::Input::X), 0.f},
                     ast::Input(ast::Input::COLOR)),
      {}};
  SystemParticle particle(Point(0.f, 0.f), .25f,
                          SystemParticle::INFINITE_LIFETIME, 0);

  SystemProcessor processor(system, rng);
  EXPECT_FALSE(processor.step(particle));
  EXPECT_EQ(particle.lifetime, 0);
  EXPECT_EQ(particle.warmup, 0);
  EXPECT_FALSE(std::isfinite(particle.x()));

  EXPECT_CALL(*rng, randomFloat(_, _)).Times(3).WillRepeatedly(Return(.5f));
  EXPECT_TRUE(processor.step(particle));
  EXPECT_FLOAT_EQ(particle.x(), 2.f);
  EXPECT_EQ(particle.lifetime, SystemParticle::INFINITE_LIFETIME);
}

}  // namespace chaoskit::core
