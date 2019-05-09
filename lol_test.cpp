#include "gtest/gtest.h"

namespace {

class LolTest : public ::testing::Test {};

TEST_F(LolTest, OneDoesNotEqualsTwo) { EXPECT_NE(1, 2); }

}  // namespace
