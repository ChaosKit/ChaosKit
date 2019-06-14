#include "Id.h"
#include <gmock/gmock.h>

using chaoskit::state::Id;

class IdTest : public ::testing::Test {};

TEST_F(IdTest, EqualIdsAreEqual) {
  Id one{12, 34};
  Id two{12, 34};

  EXPECT_EQ(one, two);
}

TEST_F(IdTest, NonEqualIdsAreNotEqual) {
  Id one{12, 34};
  Id two{34, 12};

  EXPECT_NE(one, two);
}

TEST_F(IdTest, IsHashable) {
  Id one{12, 34};
  Id two{12, 34};

  EXPECT_EQ(std::hash<Id>{}(one), std::hash<Id>{}(two));
}
