#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Store.h"

#include <gmock/gmock.h>

#include "Id.h"

using chaoskit::state::Id;
using chaoskit::state::IdTypeMismatch;
using chaoskit::state::Store;
using testing::Eq;
using testing::Field;
using testing::NotNull;
using testing::Pointee;

struct Simple {
  int property = 0;
};
struct Another {
  float field = 0;
};

class StoreTest : public ::testing::Test {};

// Store::create()

TEST_F(StoreTest, CreatesIdsForEntities) {
  Store<Simple> store;

  Id id = store.create<Simple>();

  Id expected{1, 1};
  EXPECT_EQ(expected, id);
}

TEST_F(StoreTest, AssignsUniqueIdsWithinType) {
  Store<Simple> store;

  Id one = store.create<Simple>();
  Id two = store.create<Simple>();

  EXPECT_NE(one, two);
}

TEST_F(StoreTest, AssignsUniqueTypesAcrossTypes) {
  Store<Simple, Another> store;

  Id one = store.create<Simple>();
  Id two = store.create<Another>();

  EXPECT_NE(one, two);
}

TEST_F(StoreTest, AllowsToModifyObjectAfterCreation) {
  Store<Simple> store;

  Id id = store.create<Simple>([](Simple *simple) { simple->property = 42; });

  EXPECT_THAT(store.find<Simple>(id),
              Pointee(Field(&Simple::property, Eq(42))));
}

// Store::find()

TEST_F(StoreTest, FindsEntities) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  const auto *entity = store.find<Simple>(id);

  EXPECT_THAT(entity, NotNull());
}

TEST_F(StoreTest, DoesNotFindEntitiesForInvalidIds) {
  Store<Simple> store;
  Id invalid = store.lastId<Simple>();
  store.create<Simple>();

  const auto *entity = store.find<Simple>(invalid);

  EXPECT_EQ(nullptr, entity);
}

TEST_F(StoreTest, ThrowsWhenFindingDifferentTypeThanStoredInId) {
  Store<Simple> store;
  Id id{0, 0};

  EXPECT_THROW(store.find<Simple>(id), IdTypeMismatch);
}

// Store::lastId()

TEST_F(StoreTest, ReturnsLastId) {
  Store<Simple> store;
  store.create<Simple>();
  Id second = store.create<Simple>();

  Id last = store.lastId<Simple>();

  EXPECT_EQ(second, last);
}

// Store::isValid()

TEST_F(StoreTest, RecognizesInvalidIds) {
  Store<Simple> store;
  Id invalid{0, 0};

  EXPECT_FALSE(store.isValid(invalid));
}

TEST_F(StoreTest, RecognizesZeroIdsAsInvalid) {
  Store<Simple> store;
  Id invalid = store.lastId<Simple>();

  EXPECT_FALSE(store.isValid(invalid));
}

TEST_F(StoreTest, RecognizesWrongTypesInIds) {
  Store<Simple> store;
  Id invalid{2137, 42};

  EXPECT_FALSE(store.isValid(invalid));
}

TEST_F(StoreTest, RecognizesValidIds) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  EXPECT_TRUE(store.isValid(id));
}

// Store::update()

TEST_F(StoreTest, UpdatesFields) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  bool isUpdated =
      store.update<Simple>(id, [](Simple *s) { s->property = 42; });
  auto *entity = store.find<Simple>(id);

  EXPECT_TRUE(isUpdated);
  EXPECT_THAT(entity, Pointee(Field(&Simple::property, Eq(42))));
}

TEST_F(StoreTest, DoesNotUpdateMissingEntities) {
  Store<Simple> store;
  Id missing = store.lastId<Simple>();

  bool isUpdated =
      store.update<Simple>(missing, [](Simple *s) { s->property = 42; });

  EXPECT_FALSE(isUpdated);
}

TEST_F(StoreTest, ThrowsWhenUpdatingDifferentTypeThanStoredInId) {
  Store<Simple> store;
  Id differentType{2137, 2};

  EXPECT_THROW(store.update<Simple>(differentType, [](Simple *s) {}),
               IdTypeMismatch);
}

// Store::has()

TEST_F(StoreTest, RecognizesIfIdExists) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  EXPECT_TRUE(store.has(id));
}

TEST_F(StoreTest, RecognizesIfIdDoesNotExist) {
  Store<Simple> store;
  Id missing = store.lastId<Simple>();

  EXPECT_FALSE(store.has(missing));
}

// Store::count

TEST_F(StoreTest, CountsEntitiesOfSingleType) {
  Store<Simple, Another> store;
  store.create<Simple>();
  store.create<Simple>();
  store.create<Another>();

  size_t simpleCount = store.count<Simple>();
  size_t anotherCount = store.count<Another>();

  EXPECT_EQ(2, simpleCount);
  EXPECT_EQ(1, anotherCount);
}

// Store::size()

TEST_F(StoreTest, TellsHowManyEntitiesItContains) {
  Store<Simple, Another> store;
  store.create<Simple>();
  store.create<Simple>();
  store.create<Another>();

  size_t size = store.size();

  EXPECT_EQ(3, size);
}

// Store::remove()

TEST_F(StoreTest, RemovesEntities) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  store.remove(id);

  EXPECT_FALSE(store.has(id));
}

// Store::clear()

TEST_F(StoreTest, ClearsEntitiesOfSingleType) {
  Store<Simple, Another> store;
  Id simple1 = store.create<Simple>();
  Id simple2 = store.create<Simple>();
  Id another1 = store.create<Another>();
  Id another2 = store.create<Another>();

  store.clear<Simple>();

  EXPECT_FALSE(store.has(simple1));
  EXPECT_FALSE(store.has(simple2));
  EXPECT_TRUE(store.has(another1));
  EXPECT_TRUE(store.has(another2));
}

// Store::clearAll()

TEST_F(StoreTest, ClearsEverything) {
  Store<Simple, Another> store;
  Id simple1 = store.create<Simple>();
  Id simple2 = store.create<Simple>();
  Id another1 = store.create<Another>();
  Id another2 = store.create<Another>();

  store.clearAll();

  EXPECT_FALSE(store.has(simple1));
  EXPECT_FALSE(store.has(simple2));
  EXPECT_FALSE(store.has(another1));
  EXPECT_FALSE(store.has(another2));
}

#pragma clang diagnostic pop
