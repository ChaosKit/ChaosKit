#include <gmock/gmock.h>
#include "Lifetime.h"
#include "testing/MockRng.h"

namespace chaoskit::core {

using namespace ::testing;

TEST(LifetimeTest, DefaultIsInfinite) {
  Lifetime lifetime;
  EXPECT_EQ(0, lifetime.min());
  EXPECT_EQ(Lifetime::INFINITE, lifetime.max());
  EXPECT_TRUE(lifetime.isInfinite());
}

TEST(LifetimeTest, FiniteLifetime) {
  Lifetime lifetime(10, 30);
  EXPECT_EQ(10, lifetime.min());
  EXPECT_EQ(30, lifetime.max());
  EXPECT_FALSE(lifetime.isInfinite());
}

TEST(LifetimeTest, InfiniteLifetimeWithWarmup) {
  Lifetime lifetime(15, Lifetime::INFINITE);
  EXPECT_EQ(15, lifetime.min());
  EXPECT_EQ(Lifetime::INFINITE, lifetime.max());
  EXPECT_TRUE(lifetime.isInfinite());
}

TEST(LifetimeTest, NormalizesNegativeMin) {
  bool adjusted = false;
  Lifetime lifetime = Lifetime::normalize(-5, 20, &adjusted);
  EXPECT_TRUE(adjusted);
  EXPECT_EQ(0, lifetime.min());
  EXPECT_EQ(20, lifetime.max());
}

TEST(LifetimeTest, NormalizesNonPositiveFiniteMax) {
  bool adjusted = false;
  Lifetime lifetime0 = Lifetime::normalize(0, 0, &adjusted);
  EXPECT_TRUE(adjusted);
  EXPECT_EQ(0, lifetime0.min());
  EXPECT_EQ(1, lifetime0.max());

  adjusted = false;
  Lifetime lifetimeNeg = Lifetime::normalize(0, -5, &adjusted);
  EXPECT_TRUE(adjusted);
  EXPECT_EQ(0, lifetimeNeg.min());
  EXPECT_EQ(1, lifetimeNeg.max());
}

TEST(LifetimeTest, NormalizesMinGreaterThanMaxWhenFinite) {
  bool adjusted = false;
  Lifetime lifetime = Lifetime::normalize(25, 20, &adjusted);
  EXPECT_TRUE(adjusted);
  EXPECT_EQ(20, lifetime.min());
  EXPECT_EQ(20, lifetime.max());
}

TEST(LifetimeTest, NormalizationReportsNoAdjustmentForValidInputs) {
  bool adjusted = true;
  Lifetime lifetime = Lifetime::normalize(5, 20, &adjusted);
  EXPECT_FALSE(adjusted);
  EXPECT_EQ(5, lifetime.min());
  EXPECT_EQ(20, lifetime.max());

  adjusted = true;
  Lifetime infinite = Lifetime::normalize(5, Lifetime::INFINITE, &adjusted);
  EXPECT_FALSE(adjusted);
  EXPECT_EQ(5, infinite.min());
  EXPECT_TRUE(infinite.isInfinite());
}

TEST(LifetimeTest, GeneratesInfiniteInitialLifetime) {
  Lifetime lifetime(10, Lifetime::INFINITE);
  MockRng rng;
  EXPECT_CALL(rng, randomInt(_, _)).Times(0);

  EXPECT_EQ(Lifetime::INFINITE, lifetime.generateInitialLifetime(rng));
}

TEST(LifetimeTest, GeneratesRandomInitialLifetimePerAdr0003) {
  Lifetime lifetime(10, 30);
  MockRng rng;
  EXPECT_CALL(rng, randomInt(11, 30)).WillOnce(Return(18));

  EXPECT_EQ(18, lifetime.generateInitialLifetime(rng));
}

TEST(LifetimeTest, GeneratesInitialLifetimeWhenMinEqualsMax) {
  Lifetime lifetime(20, 20);
  MockRng rng;
  EXPECT_CALL(rng, randomInt(20, 20)).WillOnce(Return(20));

  EXPECT_EQ(20, lifetime.generateInitialLifetime(rng));
}

TEST(LifetimeTest, RevivedLifetimeReturnsMax) {
  Lifetime finite(10, 30);
  EXPECT_EQ(30, finite.revivedLifetime());

  Lifetime infinite(5, Lifetime::INFINITE);
  EXPECT_EQ(Lifetime::INFINITE, infinite.revivedLifetime());
}

}  // namespace chaoskit::core
