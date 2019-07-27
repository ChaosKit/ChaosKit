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

// The hierarchy represented in these tests looks as follows:
//
//   +---> One <---+---> Two
//   |   1     *   |   *
//   |             |
//   + 1         1 + 1
// HasOne <-+-> HasMany
//        1 | 1
//          |
//        1 + 1
//         Top

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
  std::vector<const One*> ones = {};
  std::vector<const Two*> twos = {};
};
struct Top {
  HasOne* hasOne;
  HasMany* hasMany;
};

CHAOSKIT_DEFINE_RELATION(HasOne, One, &HasOne::one);
CHAOSKIT_DEFINE_RELATION(HasMany, One, &HasMany::ones);
CHAOSKIT_DEFINE_RELATION(HasMany, Two, &HasMany::twos);
CHAOSKIT_DEFINE_RELATION(Top, HasOne, &Top::hasOne);
CHAOSKIT_DEFINE_RELATION(Top, HasMany, &Top::hasMany);

CHAOSKIT_DEFINE_CHILDREN(HasOne, One);
CHAOSKIT_DEFINE_CHILDREN(HasMany, One, Two);
CHAOSKIT_DEFINE_CHILDREN(Top, HasOne, HasMany);

CHAOSKIT_DEFINE_PARENTS(One, HasOne, HasMany);
CHAOSKIT_DEFINE_PARENTS(Two, HasMany);
CHAOSKIT_DEFINE_PARENTS(HasOne, Top);
CHAOSKIT_DEFINE_PARENTS(HasMany, Top);

class HierarchyTest : public testing::Test {};

TEST_F(HierarchyTest, IsParent) {
  EXPECT_FALSE(isParent<One>());
  EXPECT_FALSE(isParent<Two>());
  EXPECT_TRUE(isParent<HasOne>());
  EXPECT_TRUE(isParent<HasMany>());
  EXPECT_TRUE(isParent<Top>());
}

TEST_F(HierarchyTest, IsChild) {
  EXPECT_TRUE(isChild<One>());
  EXPECT_TRUE(isChild<Two>());
  EXPECT_TRUE(isChild<HasOne>());
  EXPECT_TRUE(isChild<HasMany>());
  EXPECT_FALSE(isChild<Top>());
}

TEST_F(HierarchyTest, IsParentOf) {
  EXPECT_TRUE((isParentOf<One, HasOne>()));
  EXPECT_TRUE((isParentOf<One, HasMany>()));
  EXPECT_TRUE((isParentOf<HasOne, Top>()));
  EXPECT_TRUE((isParentOf<Two, HasMany>()));
  EXPECT_TRUE((isParentOf<HasMany, Top>()));

  EXPECT_FALSE((isParentOf<HasMany, HasOne>()));
  EXPECT_FALSE((isParentOf<Two, One>()));
  EXPECT_FALSE((isParentOf<Top, One>()));
}

TEST_F(HierarchyTest, IsChildOf) {
  EXPECT_TRUE((isChildOf<HasOne, One>()));
  EXPECT_TRUE((isChildOf<HasMany, One>()));
  EXPECT_TRUE((isChildOf<HasMany, Two>()));
  EXPECT_TRUE((isChildOf<Top, HasOne>()));
  EXPECT_TRUE((isChildOf<Top, HasMany>()));

  EXPECT_FALSE((isChildOf<HasOne, HasMany>()));
  EXPECT_FALSE((isChildOf<One, Two>()));
  EXPECT_FALSE((isChildOf<Top, Two>()));
}

TEST_F(HierarchyTest, IsValidRelation) {
  EXPECT_TRUE((isValidRelation<HasOne, One>()));
  EXPECT_TRUE((isValidRelation<HasMany, One>()));
  EXPECT_TRUE((isValidRelation<HasMany, Two>()));

  EXPECT_FALSE((isValidRelation<HasMany, HasOne>()));
  EXPECT_FALSE((isValidRelation<HasOne, HasMany>()));
  EXPECT_FALSE((isValidRelation<One, Two>()));
}

TEST_F(HierarchyTest, IsHasManyRelationship) {
  EXPECT_FALSE((isHasManyRelationship<HasOne, One>()));
  EXPECT_FALSE((isHasManyRelationship<Top, HasMany>()));
  EXPECT_TRUE((isHasManyRelationship<HasMany, One>()));
  EXPECT_TRUE((isHasManyRelationship<HasMany, Two>()));
}

TEST_F(HierarchyTest, IsHasOneRelationship) {
  EXPECT_TRUE((isHasOneRelationship<HasOne, One>()));
  EXPECT_TRUE((isHasOneRelationship<Top, HasMany>()));
  EXPECT_FALSE((isHasOneRelationship<HasMany, One>()));
  EXPECT_FALSE((isHasOneRelationship<HasMany, Two>()));
}

TEST_F(HierarchyTest, GetChild) {
  One child;
  HasOne parent{&child};

  EXPECT_EQ(parent.one, getChildren<One>(parent));
}

TEST_F(HierarchyTest, GetChildren) {
  One child{};
  HasMany parent{{&child}};

  EXPECT_EQ(parent.ones, getChildren<One>(parent));
}

TEST_F(HierarchyTest, GetSecondChild) {
  Two child;
  HasMany parent{{}, {&child}};

  EXPECT_EQ(parent.twos, getChildren<Two>(parent));
}

TEST_F(HierarchyTest, AssociateChildForHasOne) {
  One child;
  HasOne parent{};

  associateChild(parent, &child);

  EXPECT_THAT(parent.one, Eq(&child));
}

TEST_F(HierarchyTest, AssociateChildForHasMany) {
  One child{};
  HasMany parent{};

  associateChild(parent, &child);

  EXPECT_THAT(parent.ones, ElementsAre(&child));
}

TEST_F(HierarchyTest, DissociateChildForHasOne) {
  One child;
  HasOne parent{&child};

  dissociateChild(parent, &child);

  EXPECT_THAT(parent.one, IsNull());
}

TEST_F(HierarchyTest, DissociateChildForHasMany) {
  One child{};
  HasMany parent{{&child}};

  dissociateChild(parent, &child);

  EXPECT_THAT(parent.ones, ElementsAre());
}

TEST_F(HierarchyTest, CountOfChildren) {
  EXPECT_EQ(0, chaoskit::state::ChildrenOf<One>::count);
  EXPECT_EQ(0, chaoskit::state::ChildrenOf<Two>::count);
  EXPECT_EQ(1, chaoskit::state::ChildrenOf<HasOne>::count);
  EXPECT_EQ(2, chaoskit::state::ChildrenOf<HasMany>::count);
  EXPECT_EQ(2, chaoskit::state::ChildrenOf<Top>::count);
}

TEST_F(HierarchyTest, CountOfParents) {
  EXPECT_EQ(2, chaoskit::state::ParentsOf<One>::count);
  EXPECT_EQ(1, chaoskit::state::ParentsOf<Two>::count);
  EXPECT_EQ(1, chaoskit::state::ParentsOf<HasOne>::count);
  EXPECT_EQ(1, chaoskit::state::ParentsOf<HasMany>::count);
  EXPECT_EQ(0, chaoskit::state::ParentsOf<Top>::count);
}

class MockFn {
 public:
  MOCK_METHOD(void, callOne, ());
  MOCK_METHOD(void, callHasOne, ());
  MOCK_METHOD(void, callHasMany, ());
  MOCK_METHOD(void, callTwo, ());
  MOCK_METHOD(void, callTop, ());

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
  void operator()(HasMany*) {
    callHasMany();
  }
  template <>
  void operator()(Two*) {
    callTwo();
  }
  template <>
  void operator()(Top*) {
    callTop();
  }
};

TEST_F(HierarchyTest, ForEachWorksForOneChild) {
  MockFn fn;
  EXPECT_CALL(fn, callOne);
  EXPECT_CALL(fn, callTwo).Times(0);
  EXPECT_CALL(fn, callHasOne).Times(0);
  using Connections = chaoskit::state::ChildrenOf<HasOne>::List;

  Connections::forEach(fn);
}

TEST_F(HierarchyTest, ForEachWorksForManyChildren) {
  MockFn fn;
  EXPECT_CALL(fn, callOne);
  EXPECT_CALL(fn, callTwo);
  EXPECT_CALL(fn, callHasMany).Times(0);
  using Connections = chaoskit::state::ChildrenOf<HasMany>::List;

  Connections::forEach(fn);
}

TEST_F(HierarchyTest, ForEachWorksForOneParent) {
  MockFn fn;
  EXPECT_CALL(fn, callTwo).Times(0);
  EXPECT_CALL(fn, callHasMany);
  using Connections = chaoskit::state::ParentsOf<Two>::List;

  Connections::forEach(fn);
}

TEST_F(HierarchyTest, ForEachWorksForManyParents) {
  MockFn fn;
  EXPECT_CALL(fn, callOne).Times(0);
  EXPECT_CALL(fn, callHasOne);
  EXPECT_CALL(fn, callHasMany);
  using Connections = chaoskit::state::ParentsOf<One>::List;

  Connections::forEach(fn);
}

#pragma clang diagnostic pop
