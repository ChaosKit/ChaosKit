#include <gmock/gmock.h>

#include "ColorMapRegistry.h"
#include "errors.h"

// [[nodiscard]] issues warnings in ASSERT_THROW/EXPECT_THROW tests
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

namespace chaoskit::core {

class ColorMapRegistryTest : public testing::Test {};

using namespace testing;

TEST_F(ColorMapRegistryTest, ContainsDefaultColorMaps) {
  ColorMapRegistry registry;

  EXPECT_THAT(registry.names(),
              UnorderedElementsAre("BlackWhite", "Rainbow", "BGR"));
}

TEST_F(ColorMapRegistryTest, ReturnsValidColorMaps) {
  ColorMapRegistry registry;

  const ColorMap* blackWhite = registry.get("BlackWhite");
  ASSERT_NE(blackWhite, nullptr);

  const ColorMap* rainbow = registry.get("Rainbow");
  ASSERT_NE(rainbow, nullptr);

  const ColorMap* bgr = registry.get("BGR");
  ASSERT_NE(bgr, nullptr);
}

TEST_F(ColorMapRegistryTest, BGRColorMapValues) {
  ColorMapRegistry registry;

  const ColorMap* bgr = registry.get("BGR");
  ASSERT_NE(bgr, nullptr);

  Color c0 = bgr->map(0.f);
  EXPECT_FLOAT_EQ(c0.r, 0.f);
  EXPECT_FLOAT_EQ(c0.g, 0.f);
  EXPECT_FLOAT_EQ(c0.b, 1.f);

  Color c1 = bgr->map(0.5f);
  EXPECT_FLOAT_EQ(c1.r, 0.f);
  EXPECT_FLOAT_EQ(c1.g, 1.f);
  EXPECT_FLOAT_EQ(c1.b, 0.f);

  Color c2 = bgr->map(1.f);
  EXPECT_FLOAT_EQ(c2.r, 1.f);
  EXPECT_FLOAT_EQ(c2.g, 0.f);
  EXPECT_FLOAT_EQ(c2.b, 0.f);
}

TEST_F(ColorMapRegistryTest, ThrowsOnMissingColorMap) {
  ColorMapRegistry registry;

  EXPECT_THROW(registry.get("NonExistent"), InvalidColorMap);
}

}  // namespace chaoskit::core
