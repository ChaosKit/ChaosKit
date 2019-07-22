#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "hierarchy.h"

#include <gmock/gmock.h>
#include <vector>

using chaoskit::state::areRelated;
using chaoskit::state::associateChild;
using chaoskit::state::dissociateChild;
using chaoskit::state::getChildren;
using chaoskit::state::hasManyChildren;
using chaoskit::state::hasOneChild;
using chaoskit::state::isChild;
using chaoskit::state::isChildOf;
using chaoskit::state::isParent;
using chaoskit::state::isParentOf;
using testing::ElementsAre;
using testing::Eq;
using testing::Field;
using testing::IsNull;
using testing::Pointee;

struct One {
  int one = 1;
};
struct HasOne {
  One* one;
};
struct HasMany {
  std::vector<const HasOne*> hasOnes = {};
};

DEFINE_RELATIONSHIP(HasOne, One, &HasOne::one);
DEFINE_RELATIONSHIP(HasMany, HasOne, &HasMany::hasOnes);

class HierarchyTest : public testing::Test {};

TEST_F(HierarchyTest, IsParent) {
  EXPECT_FALSE(isParent<One>());
  EXPECT_TRUE(isParent<HasOne>());
  EXPECT_TRUE(isParent<HasMany>());
}

TEST_F(HierarchyTest, IsChild) {
  EXPECT_TRUE(isChild<One>());
  EXPECT_TRUE(isChild<HasOne>());
  EXPECT_FALSE(isChild<HasMany>());
}

TEST_F(HierarchyTest, IsParentOf) {
  EXPECT_TRUE((isParentOf<One, HasOne>()));
  EXPECT_TRUE((isParentOf<HasOne, HasMany>()));
  EXPECT_FALSE((isParentOf<One, HasMany>()));
  EXPECT_FALSE((isParentOf<HasMany, HasOne>()));
}

TEST_F(HierarchyTest, IsChildOf) {
  EXPECT_TRUE((isChildOf<HasOne, One>()));
  EXPECT_TRUE((isChildOf<HasMany, HasOne>()));
  EXPECT_FALSE((isChildOf<HasMany, One>()));
  EXPECT_FALSE((isChildOf<HasOne, HasMany>()));
}

TEST_F(HierarchyTest, AreRelated) {
  EXPECT_TRUE((areRelated<HasOne, One>()));
  EXPECT_TRUE((areRelated<HasMany, HasOne>()));
  EXPECT_FALSE((areRelated<HasMany, One>()));
  EXPECT_TRUE((areRelated<HasOne, HasMany>()));
}

TEST_F(HierarchyTest, HasManyChildren) {
  EXPECT_FALSE(hasManyChildren<HasOne>());
  EXPECT_TRUE(hasManyChildren<HasMany>());
}

TEST_F(HierarchyTest, HasOneChild) {
  EXPECT_TRUE(hasOneChild<HasOne>());
  EXPECT_FALSE(hasOneChild<HasMany>());
}

TEST_F(HierarchyTest, GetChild) {
  One child;
  HasOne parent{&child};

  EXPECT_EQ(parent.one, getChildren(parent));
}

TEST_F(HierarchyTest, GetChildren) {
  HasOne child{};
  HasMany parent{{&child}};

  EXPECT_EQ(parent.hasOnes, getChildren(parent));
}

TEST_F(HierarchyTest, AssociateChildForHasOne) {
  One child;
  HasOne parent{};

  associateChild(parent, &child);

  EXPECT_THAT(parent, Field(&HasOne::one, Eq(&child)));
}

TEST_F(HierarchyTest, AssociateChildForHasMany) {
  HasOne child{};
  HasMany parent{};

  associateChild(parent, &child);

  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre(&child)));
}

TEST_F(HierarchyTest, DissociateChildForHasOne) {
  One child;
  HasOne parent{&child};

  dissociateChild(parent, &child);

  EXPECT_THAT(parent, Field(&HasOne::one, IsNull()));
}

TEST_F(HierarchyTest, DissociateChildForHasMany) {
  HasOne child{};
  HasMany parent{{&child}};

  dissociateChild(parent, &child);

  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre()));
}

#pragma clang diagnostic pop
