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

SystemParticle createParticle(float x = 0.f, float y = 0.f, float color = 0.f) {
  return SystemParticle{{Point(x, y), color, Particle::IMMORTAL, 0}};
}

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
  processor.setParticleLifetime(30);
  SystemParticle actual = processor.createParticle();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.particle.ttl = 15;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, CreatesParticleWithSkipping) {
  // (output.x, output.y) = (1, 2)
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f})};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(.5f));

  SystemProcessor processor(system, rng);
  processor.setSkip(42);
  SystemParticle actual = processor.createParticle();

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.particle.skip = 42;
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
  input.particle.ttl = 0;

  SystemProcessor processor(system, rng);
  processor.setParticleLifetime(30);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(.5f, .5f, .5f);
  expected.particle.ttl = 29;
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, SkipsSteps) {
  // (output.x, output.y) = (input.x + 1, input.y + 1)
  TransformSystem system{ast::Transform(
      ast::Formula{ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::X), 1.f),
                   ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::Y), 1.f)})};
  SystemParticle input = createParticle();
  input.particle.skip = 2;

  SystemProcessor processor(system, rng);
  processor.setSkip(2);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(3, 3);
  ASSERT_EQ(expected, actual);
}

TEST_F(SystemProcessorTest, SkipDoesNotExceedLifetime) {
  // (output.x, output.y) = (input.x + 1, input.y + 1)
  TransformSystem system{ast::Transform(
      ast::Formula{ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::X), 1.f),
                   ast::BinaryFunction(ast::BinaryFunction::ADD,
                                       ast::Input(ast::Input::Y), 1.f)})};
  SystemParticle input = createParticle();
  input.particle.ttl = 2;
  input.particle.skip = 4;

  SystemProcessor processor(system, rng);
  processor.setParticleLifetime(2);
  processor.setSkip(4);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(2, 2);
  expected.particle.ttl = 0;
  expected.particle.skip = 0;
  ASSERT_EQ(expected, actual);
}

}  // namespace chaoskit::core
