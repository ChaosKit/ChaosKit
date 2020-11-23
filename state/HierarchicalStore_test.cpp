#include "HierarchicalStore.h"

#include <gmock/gmock.h>
#include <vector>

#include "Id.h"

using chaoskit::state::HierarchicalStore;
using chaoskit::state::Id;
using chaoskit::state::IdTypeMismatchError;
using chaoskit::state::MissingIdError;
using testing::ElementsAre;
using testing::Eq;
using testing::Field;
using testing::IsNull;
using testing::NotNull;
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

class HierarchicalStoreTest : public testing::Test {};

// HierarchicalStore::associateChildWith()

TEST_F(HierarchicalStoreTest, AssociatesExistingChildInHasOneRelation) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.create<One>();

  store.associateChildWith<HasOne, One>(parentId, childId);

  const auto* parent = store.find<HasOne>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent->one, Eq(child));
}

TEST_F(HierarchicalStoreTest, AssociatesExistingChildInHasManyRelation) {
  HierarchicalStore<One, HasMany> store;
  Id parentId = store.create<HasMany>();
  Id childId = store.create<One>();

  store.associateChildWith<HasMany, One>(parentId, childId);

  const auto* parent = store.find<HasMany>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent->ones, ElementsAre(child));
}

TEST_F(HierarchicalStoreTest,
       AssociateChildWithThrowsWhenParentTypeDoesNotMatchId) {
  HierarchicalStore<HasOne, One> store;
  Id notParentId = store.create<One>();
  Id childId = store.create<One>();

  EXPECT_THROW((store.associateChildWith<HasOne, One>(notParentId, childId)),
               IdTypeMismatchError);
}

TEST_F(HierarchicalStoreTest,
       AssociateChildWithThrowsWhenChildTypeDoesNotMatchId) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id notChildId = store.create<HasOne>();

  EXPECT_THROW((store.associateChildWith<HasOne, One>(parentId, notChildId)),
               IdTypeMismatchError);
}

TEST_F(HierarchicalStoreTest, AssociateChildWithThrowsWhenChildIsMissing) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.lastId<One>();

  EXPECT_THROW((store.associateChildWith<HasOne, One>(parentId, childId)),
               MissingIdError);
}

// HierarchicalStore::associateNewChildWith()

TEST_F(HierarchicalStoreTest, CreatesNewChild) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();

  store.associateNewChildWith<HasOne, One>(parentId);

  EXPECT_EQ(1, store.count<One>());
}

TEST_F(HierarchicalStoreTest, CreatesAndUpdatesNewChild) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();

  Id childId = store.associateNewChildWith<HasOne, One>(
      parentId, [](One* one) { one->one = 111; });

  const auto* child = store.find<One>(childId);
  EXPECT_EQ(111, child->one);
}

TEST_F(HierarchicalStoreTest, AssociatesNewChildInHasOneRelation) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();

  store.associateNewChildWith<HasOne, One>(parentId);

  const Id childId = store.lastId<One>();
  const auto* parent = store.find<HasOne>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent->one, Eq(child));
}

TEST_F(HierarchicalStoreTest, AssociatesNewChildInHasManyRelation) {
  HierarchicalStore<One, HasMany> store;
  Id parentId = store.create<HasMany>();

  store.associateNewChildWith<HasMany, One>(parentId);

  const Id childId = store.lastId<One>();
  const auto* parent = store.find<HasMany>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent->ones, ElementsAre(child));
}

TEST_F(HierarchicalStoreTest,
       AssociateNewChildWithThrowsWhenParentTypeDoesNotMatchId) {
  HierarchicalStore<HasOne, One> store;
  Id notParentId = store.create<One>();

  EXPECT_THROW((store.associateNewChildWith<HasOne, One>(notParentId)),
               IdTypeMismatchError);
}

// HierarchicalStore::dissociateChildFrom()

TEST_F(HierarchicalStoreTest, DissociatesChildFromParent) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.associateNewChildWith<HasOne, One>(parentId);

  store.dissociateChildFrom<HasOne, One>(parentId, childId);

  const auto* parent = store.find<HasOne>(parentId);
  EXPECT_THAT(parent->one, IsNull());
}

TEST_F(HierarchicalStoreTest, DissociatingDoesNotRemoveElements) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.associateNewChildWith<HasOne, One>(parentId);

  store.dissociateChildFrom<HasOne, One>(parentId, childId);

  const auto* child = store.find<One>(childId);
  EXPECT_THAT(child, NotNull());
}

// HierarchicalStore::remove()

TEST_F(HierarchicalStoreTest, RemovesEntitiesWithoutHavingToPassType) {
  HierarchicalStore<HasOne, One> store;
  Id id = store.create<One>();

  store.remove(id);

  EXPECT_FALSE(store.has(id));
}

TEST_F(HierarchicalStoreTest, TypelessRemoveWorksRecursively) {
  HierarchicalStore<HasOne, One> store;
  Id id = store.create<HasOne>();
  store.associateNewChildWith<HasOne, One>(id);

  store.remove(id);

  EXPECT_EQ(0, store.size());
}

// HierarchicalStore::removeWithType()

TEST_F(HierarchicalStoreTest, RemovesEntities) {
  HierarchicalStore<HasOne, One> store;
  Id id = store.create<One>();

  store.removeWithType<One>(id);

  EXPECT_FALSE(store.has(id));
}

TEST_F(HierarchicalStoreTest, ThrowsWhenRemovedIdTypeDoesNotMatch) {
  HierarchicalStore<HasOne, One> store;
  Id id = store.create<One>();

  EXPECT_THROW(store.removeWithType<HasOne>(id), IdTypeMismatchError);
}

TEST_F(HierarchicalStoreTest, RemoveDissociatesChildFromParentInHasOneCase) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.create<One>();
  store.associateChildWith<HasOne, One>(parentId, childId);

  store.removeWithType<One>(childId);

  const auto* parent = store.find<HasOne>(parentId);
  EXPECT_THAT(parent->one, IsNull());
}

TEST_F(HierarchicalStoreTest, RemoveDissociatesChildFromParentInHasManyCase) {
  HierarchicalStore<One, HasMany> store;
  Id parentId = store.create<HasMany>();
  Id childId = store.create<One>();
  store.associateChildWith<HasMany, One>(parentId, childId);

  store.removeWithType<One>(childId);

  const auto* parent = store.find<HasMany>(parentId);
  EXPECT_THAT(parent->ones, ElementsAre());
}

TEST_F(HierarchicalStoreTest, RemoveDissociatesChildFromAllParents) {
  HierarchicalStore<One, HasOne, HasMany> store;
  Id hasOneId = store.create<HasOne>();
  Id hasManyId = store.create<HasMany>();
  Id oneId = store.create<One>();
  store.associateChildWith<HasOne, One>(hasOneId, oneId);
  store.associateChildWith<HasMany, One>(hasManyId, oneId);

  store.removeWithType<One>(oneId);

  const auto* hasOne = store.find<HasOne>(hasOneId);
  const auto* hasMany = store.find<HasMany>(hasManyId);
  EXPECT_THAT(hasOne->one, IsNull());
  EXPECT_THAT(hasMany->ones, ElementsAre());
}

TEST_F(HierarchicalStoreTest, RecursivelyRemovesChildren) {
  HierarchicalStore<One, HasOne, HasMany, Two, Top> store;
  Id top = store.create<Top>();
  Id middleOne = store.create<HasOne>();
  Id middleMany = store.create<HasMany>();
  Id bottom = store.create<One>();
  store.associateChildWith<Top, HasOne>(top, middleOne);
  store.associateChildWith<Top, HasMany>(top, middleMany);
  store.associateNewChildWith<HasMany, Two>(middleMany);
  store.associateChildWith<HasOne, One>(middleOne, bottom);

  store.removeWithType<Top>(top);

  EXPECT_EQ(0, store.size());
}
