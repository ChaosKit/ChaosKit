#include <gmock/gmock.h>
#include "ColorMapRegistry.h"

namespace chaoskit::ui {

using namespace ::testing;

TEST(UIColorMapRegistryTest, NamesAndDefaultName) {
  ColorMapRegistry registry;
  EXPECT_THAT(registry.names(), ElementsAre("BGR", "BlackWhite", "Rainbow"));
  EXPECT_EQ(registry.defaultName(), "BGR");
}

}  // namespace chaoskit::ui
