#include <gmock/gmock.h>
#include "CameraSystemProcessor.h"
#include "SystemParticle.h"
#include "testing/MockRng.h"

namespace chaoskit::core {

using namespace ::testing;

class CameraSystemProcessorTest : public Test {
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

TEST_F(CameraSystemProcessorTest, IsSystemProcessor) {
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f}, .25f)};
  SystemParticle input = createParticle();

  CameraSystemProcessor processor(system, rng);
  SystemParticle actual = processor.process(input);

  SystemParticle expected = createParticle(1.f, 2.f, .25f);
  ASSERT_EQ(expected, actual);
}

TEST_F(CameraSystemProcessorTest, ProcessesCamera) {
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f}, .25f)};
  TransformSystem camera{ast::Transform(ast::Formula{2.f, 3.f})};
  SystemParticle input = createParticle();

  CameraSystemProcessor processor(CameraSystem{system, camera}, rng);
  SystemParticle actual = processor.processCamera(processor.process(input));

  SystemParticle expected = createParticle(2.f, 3.f, .25f);
  ASSERT_EQ(expected, actual);
}

TEST_F(CameraSystemProcessorTest, ReturnsInputWhenCameraIsUnset) {
  TransformSystem system{ast::Transform(ast::Formula{1.f, 2.f}, .25f)};
  SystemParticle input = createParticle();

  CameraSystemProcessor processor(CameraSystem{system}, rng);
  SystemParticle actual = processor.processCamera(input);

  SystemParticle expected = input;
  ASSERT_EQ(expected, actual);
}

TEST_F(CameraSystemProcessorTest, AppliesLifetimeFromCameraSystem) {
  TransformSystem system = {ast::Transform(ast::Formula{1.f, 2.f})};
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(.5f));
  EXPECT_CALL(*rng, randomInt(6, 20)).WillOnce(Return(12));

  CameraSystem cameraSystem = {system, stdx::nullopt, Lifetime(5, 20)};
  CameraSystemProcessor processor(cameraSystem, rng);

  SystemParticle created = processor.createParticle();
  EXPECT_EQ(12, created.lifetime);
  EXPECT_EQ(5, created.warmup);

  EXPECT_CALL(*rng, randomInt(11, 40)).WillOnce(Return(25));
  processor.setCameraSystem(
      CameraSystem{system, stdx::nullopt, Lifetime(10, 40)});

  SystemParticle updated = processor.createParticle();
  EXPECT_EQ(25, updated.lifetime);
  EXPECT_EQ(10, updated.warmup);
}

}  // namespace chaoskit::core
