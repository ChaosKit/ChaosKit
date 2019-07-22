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
struct HasOne {
  One* one;
};
struct HasMany {
  std::vector<const HasOne*> hasOnes = {};
};

DEFINE_RELATIONSHIP(HasOne, One, &HasOne::one);
DEFINE_RELATIONSHIP(HasMany, HasOne, &HasMany::hasOnes);

class HierarchicalStoreTest : public testing::Test {};

// HierarchicalStore::associateChildWith()

TEST_F(HierarchicalStoreTest, AssociatesExistingChildInHasOneRelation) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.create<One>();

  store.associateChildWith<HasOne>(parentId, childId);

  const auto* parent = store.find<HasOne>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent, Field(&HasOne::one, Eq(child)));
}

TEST_F(HierarchicalStoreTest, AssociatesExistingChildInHasManyRelation) {
  HierarchicalStore<HasOne, HasMany> store;
  Id parentId = store.create<HasMany>();
  Id childId = store.create<HasOne>();

  store.associateChildWith<HasMany>(parentId, childId);

  const auto* parent = store.find<HasMany>(parentId);
  const auto* child = store.find<HasOne>(childId);
  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre(child)));
}

TEST_F(HierarchicalStoreTest,
       AssociateChildWithThrowsWhenParentTypeDoesNotMatchId) {
  HierarchicalStore<HasOne, One> store;
  Id notParentId = store.create<One>();
  Id childId = store.create<One>();

  EXPECT_THROW(store.associateChildWith<HasOne>(notParentId, childId),
               IdTypeMismatchError);
}

TEST_F(HierarchicalStoreTest,
       AssociateChildWithThrowsWhenChildTypeDoesNotMatchId) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id notChildId = store.create<HasOne>();

  EXPECT_THROW(store.associateChildWith<HasOne>(parentId, notChildId),
               IdTypeMismatchError);
}

TEST_F(HierarchicalStoreTest, AssociateChildWithThrowsWhenChildIsMissing) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();
  Id childId = store.lastId<One>();

  EXPECT_THROW(store.associateChildWith<HasOne>(parentId, childId),
               MissingIdError);
}

// HierarchicalStore::associateNewChildWith()

TEST_F(HierarchicalStoreTest, CreatesNewChild) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();

  store.associateNewChildWith<HasOne>(parentId);

  EXPECT_EQ(1, store.count<One>());
}

TEST_F(HierarchicalStoreTest, AssociatesNewChildInHasOneRelation) {
  HierarchicalStore<HasOne, One> store;
  Id parentId = store.create<HasOne>();

  store.associateNewChildWith<HasOne>(parentId);

  const Id childId = store.lastId<One>();
  const auto* parent = store.find<HasOne>(parentId);
  const auto* child = store.find<One>(childId);
  EXPECT_THAT(parent, Field(&HasOne::one, Eq(child)));
}

TEST_F(HierarchicalStoreTest, AssociatesNewChildInHasManyRelation) {
  HierarchicalStore<HasOne, HasMany> store;
  Id parentId = store.create<HasMany>();

  store.associateNewChildWith<HasMany>(parentId);

  const Id childId = store.lastId<HasOne>();
  const auto* parent = store.find<HasMany>(parentId);
  const auto* child = store.find<HasOne>(childId);
  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre(child)));
}

TEST_F(HierarchicalStoreTest,
       AssociateNewChildWithThrowsWhenParentTypeDoesNotMatchId) {
  HierarchicalStore<HasOne, One> store;
  Id notParentId = store.create<One>();

  EXPECT_THROW(store.associateNewChildWith<HasOne>(notParentId),
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
  store.associateChildWith<HasOne>(parentId, childId);

  store.remove<One>(childId);

  const auto* parent = store.find<HasOne>(parentId);
  EXPECT_THAT(parent, Field(&HasOne::one, IsNull()));
}

TEST_F(HierarchicalStoreTest, RemoveDissociatesChildFromParentInHasManyCase) {
  HierarchicalStore<HasOne, HasMany> store;
  Id parentId = store.create<HasMany>();
  Id childId = store.create<HasOne>();
  store.associateChildWith<HasMany>(parentId, childId);

  store.remove<HasOne>(childId);

  const auto* parent = store.find<HasMany>(parentId);
  EXPECT_THAT(parent, Field(&HasMany::hasOnes, ElementsAre()));
}

TEST_F(HierarchicalStoreTest, RecursivelyRemovesChildren) {
  HierarchicalStore<One, HasOne, HasMany> store;
  Id top = store.create<HasMany>();
  Id middle = store.create<HasOne>();
  Id bottom = store.create<One>();
  store.associateChildWith<HasMany>(top, middle);
  store.associateChildWith<HasOne>(middle, bottom);

  store.remove<HasMany>(top);

  EXPECT_EQ(0, store.size());
}

#pragma clang diagnostic pop
