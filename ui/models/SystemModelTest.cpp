#include <gmock/gmock.h>
#include <QSignalSpy>
#include "ModelFactory.h"
#include "SystemModel.h"
#include "core/SystemParticle.h"

namespace chaoskit::ui {

using namespace ::testing;

class SystemModelTest : public Test {
 protected:
  void SetUp() override {
    systemProto_ = std::make_unique<System>();
    systemProto_->set_min_lifetime(0);
    systemProto_->set_max_lifetime(core::SystemParticle::INFINITE_LIFETIME);

    model_ = std::make_unique<SystemModel>(&factory_);
    model_->setProto(systemProto_.get());
  }

  ModelFactory factory_;
  std::unique_ptr<System> systemProto_;
  std::unique_ptr<SystemModel> model_;
};

TEST_F(SystemModelTest, CanEditMinLifetimeWhenLifetimeIsInfinite) {
  ASSERT_TRUE(model_->isInfiniteLifetime());

  QSignalSpy spy(model_.get(), &SystemModel::minLifetimeChanged);
  model_->setMinLifetime(15);

  EXPECT_EQ(15, model_->minLifetime());
  EXPECT_EQ(1, spy.size());
}

TEST_F(SystemModelTest, RejectsNegativeMinLifetime) {
  QSignalSpy spy(model_.get(), &SystemModel::minLifetimeChanged);
  model_->setMinLifetime(-1);

  EXPECT_EQ(0, model_->minLifetime());
  EXPECT_EQ(0, spy.size());
}

TEST_F(SystemModelTest, RejectsMinLifetimeGreaterThanMaxLifetimeWhenFinite) {
  model_->setMaxLifetime(30);
  ASSERT_EQ(30, model_->maxLifetime());

  QSignalSpy spy(model_.get(), &SystemModel::minLifetimeChanged);
  model_->setMinLifetime(35);

  EXPECT_EQ(0, model_->minLifetime());
  EXPECT_EQ(0, spy.size());

  model_->setMinLifetime(20);
  EXPECT_EQ(20, model_->minLifetime());
  EXPECT_EQ(1, spy.size());
}

TEST_F(SystemModelTest, RejectsMaxLifetimeLessThanOneUnlessInfinite) {
  model_->setMaxLifetime(20);
  ASSERT_EQ(20, model_->maxLifetime());

  QSignalSpy spy(model_.get(), &SystemModel::maxLifetimeChanged);
  model_->setMaxLifetime(0);
  EXPECT_EQ(20, model_->maxLifetime());
  EXPECT_EQ(0, spy.size());

  model_->setMaxLifetime(-5);
  EXPECT_EQ(20, model_->maxLifetime());
  EXPECT_EQ(0, spy.size());

  model_->setMaxLifetime(core::SystemParticle::INFINITE_LIFETIME);
  EXPECT_EQ(core::SystemParticle::INFINITE_LIFETIME, model_->maxLifetime());
  EXPECT_TRUE(model_->isInfiniteLifetime());
  EXPECT_EQ(1, spy.size());
}

TEST_F(SystemModelTest, ClampsMinLifetimeWhenMaxLifetimeReducedBelowMin) {
  model_->setMaxLifetime(50);
  model_->setMinLifetime(30);

  QSignalSpy minSpy(model_.get(), &SystemModel::minLifetimeChanged);
  QSignalSpy maxSpy(model_.get(), &SystemModel::maxLifetimeChanged);

  model_->setMaxLifetime(20);

  EXPECT_EQ(20, model_->maxLifetime());
  EXPECT_EQ(20, model_->minLifetime());
  EXPECT_EQ(1, maxSpy.size());
  EXPECT_EQ(1, minSpy.size());
}

}  // namespace chaoskit::ui
