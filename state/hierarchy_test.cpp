#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "hierarchy.h"

#include <gmock/gmock.h>
#include <vector>

using chaoskit::state::associateChild;
using chaoskit::state::dissociateChild;
using chaoskit::state::getChildren;
using chaoskit::state::isChild;
using chaoskit::state::isChildOf;
using chaoskit::state::isHasManyRelationship;
using chaoskit::state::isHasOneRelationship;
using chaoskit::state::isParent;
using chaoskit::state::isParentOf;
using chaoskit::state::isValidRelation;
using testing::ElementsAre;
using testing::Eq;
using testing::Field;
using testing::IsNull;
using testing::Pointee;

struct One {
  int one = 1;
};
struct Two {
  int two = 2;
};
struct HasOne {
  One* one;
};
struct HasMany {
  std::vector<const HasOne*> hasOnes = {};
  Two* two;
};

DEFINE_RELATION(HasOne, One, &HasOne::one);
DEFINE_RELATION(HasMany, HasOne, &HasMany::hasOnes);
DEFINE_RELATION(HasMany, Two, &HasMany::two);

DEFINE_CHILDREN(HasOne, One);
DEFINE_CHILDREN(HasMany, HasOne, Two);

class HierarchyTest : public testing::Test {};

TEST_F(HierarchyTest, IsParent) {
  EXPECT_FALSE(isParent<One>());
  EXPECT_TRUE(isParent<HasOne>());
  EXPECT_TRUE(isParent<HasMany>());
}

TEST_F(HierarchyTest, IsChild) {
  EXPECT_TRUE(isChild<One>());
  EXPECT_TRUE(isChild<Two>());
  EXPECT_TRUE(isChild<HasOne>());
  EXPECT_FALSE(isChild<HasMany>());
}

TEST_F(HierarchyTest, IsParentOf) {
  EXPECT_TRUE((isParentOf<One, HasOne>()));
  EXPECT_TRUE((isParentOf<HasOne, HasMany>()));
  EXPECT_TRUE((isParentOf<Two, HasMany>()));
  EXPECT_FALSE((isParentOf<One, HasMany>()));
  EXPECT_FALSE((isParentOf<HasMany, HasOne>()));
  EXPECT_FALSE((isParentOf<Two, One>()));
}

TEST_F(HierarchyTest, IsChildOf) {
  EXPECT_TRUE((isChildOf<HasOne, One>()));
  EXPECT_TRUE((isChildOf<HasMany, HasOne>()));
  EXPECT_TRUE((isChildOf<HasMany, Two>()));
  EXPECT_FALSE((isChildOf<HasMany, One>()));
  EXPECT_FALSE((isChildOf<HasOne, HasMany>()));
  EXPECT_FALSE((isChildOf<One, Two>()));
}

TEST_F(HierarchyTest, IsValidRelation) {
  EXPECT_TRUE((isValidRelation<HasOne, One>()));
  EXPECT_TRUE((isValidRelation<HasMany, HasOne>()));
  EXPECT_TRUE((isValidRelation<HasMany, Two>()));
  EXPECT_FALSE((isValidRelation<HasMany, One>()));
  EXPECT_FALSE((isValidRelation<HasOne, HasMany>()));
  EXPECT_FALSE((isValidRelation<One, Two>()));
}

TEST_F(HierarchyTest, IsHasManyRelationship) {
  EXPECT_FALSE((isHasManyRelationship<HasOne, One>()));
  EXPECT_FALSE((isHasManyRelationship<HasMany, Two>()));
  EXPECT_TRUE((isHasManyRelationship<HasMany, HasOne>()));
}

TEST_F(HierarchyTest, IsHasOneRelationship) {
  EXPECT_TRUE((isHasOneRelationship<HasOne, One>()));
  EXPECT_TRUE((isHasOneRelationship<HasMany, Two>()));
  EXPECT_FALSE((isHasOneRelationship<HasMany, HasOne>()));
}

TEST_F(HierarchyTest, GetChild) {
  One child;
  HasOne parent{&child};

  EXPECT_EQ(parent.one, getChildren<One>(parent));
}

TEST_F(HierarchyTest, GetChildren) {
  HasOne child{};
  HasMany parent{{&child}};

  EXPECT_EQ(parent.hasOnes, getChildren<HasOne>(parent));
}

TEST_F(HierarchyTest, GetSecondChild) {
  Two child;
  HasMany parent{{}, &child};

  EXPECT_EQ(parent.two, getChildren<Two>(parent));
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

TEST_F(HierarchyTest, CountOfChildren) {
  EXPECT_EQ(0, chaoskit::state::ChildrenOf<One>::count);
  EXPECT_EQ(1, chaoskit::state::ChildrenOf<HasOne>::count);
  EXPECT_EQ(2, chaoskit::state::ChildrenOf<HasMany>::count);
}

class MockFn {
 public:
  MOCK_METHOD(void, callOne, ());
  MOCK_METHOD(void, callHasOne, ());
  MOCK_METHOD(void, callTwo, ());

  template <typename T>
  void operator()(T) {}

  template <>
  void operator()(One*) {
    callOne();
  }
  template <>
  void operator()(HasOne*) {
    callHasOne();
  }
  template <>
  void operator()(Two*) {
    callTwo();
  }
};

TEST_F(HierarchyTest, ForEachChildWorksForOneChild) {
  MockFn fn;
  EXPECT_CALL(fn, callOne);
  EXPECT_CALL(fn, callHasOne).Times(0);
  EXPECT_CALL(fn, callTwo).Times(0);
  using ChildList = chaoskit::state::ChildrenOf<HasOne>::List;

  ChildList::forEachChild(fn);
}

TEST_F(HierarchyTest, ForEachChildWorksForManyChildren) {
  MockFn fn;
  EXPECT_CALL(fn, callOne).Times(0);
  EXPECT_CALL(fn, callHasOne);
  EXPECT_CALL(fn, callTwo);
  using ChildList = chaoskit::state::ChildrenOf<HasMany>::List;

  ChildList::forEachChild(fn);
}

#pragma clang diagnostic pop
