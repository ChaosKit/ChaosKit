#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

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

CHAOSKIT_DEFINE_RELATION(HasOne, One, &HasOne::one);
CHAOSKIT_DEFINE_RELATION(HasMany, HasOne, &HasMany::hasOnes);
CHAOSKIT_DEFINE_RELATION(HasMany, Two, &HasMany::two);

CHAOSKIT_DEFINE_CHILDREN(HasOne, One);
CHAOSKIT_DEFINE_CHILDREN(HasMany, HasOne, Two);

class HierarchicalStoreTest : public testing::Test {};

// HierarchicalStore::associateChildWith()

TEST_F(HierarchicalStoreTest, AssociatesExistingChildInHasOneRelation) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.create<One>();

  store.associateChildWith<HasOne, One>(parentId, childId);

  const auto* parent = store.find<HasOne>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent, Field(&HasOne::one, Eq(child)));
}

TEST_F(HierarchicalStoreTest, AssociatesExistingChildInHasManyRelation) {
  HierarchicalStore<HasOne, HasMany> store;
  Id parentId = store.create<HasMany>();
  Id childId = store.create<HasOne>();

  store.associateChildWith<HasMany, HasOne>(parentId, childId);

  const auto* parent = store.find<HasMany>(parentId);
  const auto* child = store.find<HasOne>(childId);
  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre(child)));
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

TEST_F(HierarchicalStoreTest, AssociatesNewChildInHasOneRelation) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();

  store.associateNewChildWith<HasOne, One>(parentId);

  const Id childId = store.lastId<One>();
  const auto* parent = store.find<HasOne>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent, Field(&HasOne::one, Eq(child)));
}

TEST_F(HierarchicalStoreTest, AssociatesNewChildInHasManyRelation) {
  HierarchicalStore<HasOne, HasMany> store;
  Id parentId = store.create<HasMany>();

  store.associateNewChildWith<HasMany, HasOne>(parentId);

  const Id childId = store.lastId<HasOne>();
  const auto* parent = store.find<HasMany>(parentId);
  const auto* child = store.find<HasOne>(childId);
  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre(child)));
}

TEST_F(HierarchicalStoreTest,
       AssociateNewChildWithThrowsWhenParentTypeDoesNotMatchId) {
  HierarchicalStore<HasOne, One> store;
  Id notParentId = store.create<One>();

  EXPECT_THROW((store.associateNewChildWith<HasOne, One>(notParentId)),
               IdTypeMismatchError);
}

// HierarchicalStore::remove()

TEST_F(HierarchicalStoreTest, RemovesEntities) {
  HierarchicalStore<HasOne, One> store;
  Id id = store.create<One>();

  store.remove<One>(id);

  EXPECT_FALSE(store.has(id));
}

TEST_F(HierarchicalStoreTest, ThrowsWhenRemovedIdTypeDoesNotMatch) {
  HierarchicalStore<HasOne, One> store;
  Id id = store.create<One>();

  EXPECT_THROW(store.remove<HasOne>(id), IdTypeMismatchError);
}

TEST_F(HierarchicalStoreTest, RemoveDissociatesChildFromParentInHasOneCase) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.create<One>();
  store.associateChildWith<HasOne, One>(parentId, childId);

  store.remove<One>(childId);

  const auto* parent = store.find<HasOne>(parentId);
  EXPECT_THAT(parent, Field(&HasOne::one, IsNull()));
}

TEST_F(HierarchicalStoreTest, RemoveDissociatesChildFromParentInHasManyCase) {
  HierarchicalStore<HasOne, HasMany> store;
  Id parentId = store.create<HasMany>();
  Id childId = store.create<HasOne>();
  store.associateChildWith<HasMany, HasOne>(parentId, childId);

  store.remove<HasOne>(childId);

  const auto* parent = store.find<HasMany>(parentId);
  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre()));
}

TEST_F(HierarchicalStoreTest, RecursivelyRemovesChildren) {
  HierarchicalStore<One, HasOne, HasMany, Two> store;
  Id top = store.create<HasMany>();
  Id middle = store.create<HasOne>();
  Id bottom = store.create<One>();
  store.associateChildWith<HasMany, HasOne>(top, middle);
  store.associateNewChildWith<HasMany, Two>(top);
  store.associateChildWith<HasOne, One>(middle, bottom);

  store.remove<HasMany>(top);

  EXPECT_EQ(0, store.size());
}

#pragma clang diagnostic pop
