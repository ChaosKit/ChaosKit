#include <gmock/gmock.h>

#include "TransformIndex.h"
#include "errors.h"

// [[nodiscard]] issues warnings in ASSERT_THROW tests
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

namespace chaoskit::core {

class TransformIndexTest : public testing::Test {};

using namespace testing;

TEST_F(TransformIndexTest, ThrowsWhenTryingToConstructTooLargeIndex) {
  ASSERT_THROW((TransformIndex{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                               15, 16, 17}),
               InvalidTransformIndex);
}

TEST_F(TransformIndexTest, SameIndicesAreEqual) {
  TransformIndex one{1, 2, 3};
  TransformIndex two{1, 2, 3};

  ASSERT_EQ(one, two);
  ASSERT_EQ(two, one);
}

TEST_F(TransformIndexTest, IndicesAtDifferentLevelsAreNotEqual) {
  TransformIndex one{1, 2};
  TransformIndex two{1, 2, 3};

  ASSERT_NE(one, two);
  ASSERT_NE(two, one);
}

TEST_F(TransformIndexTest, IndicesWithDifferentValuesAreNotEqual) {
  TransformIndex one{1, 2};
  TransformIndex two{1, 3};

  ASSERT_NE(one, two);
  ASSERT_NE(two, one);
}

TEST_F(TransformIndexTest, IndicesWithValueAreNotEqualToRoot) {
  TransformIndex root;
  TransformIndex index{1, 2};

  ASSERT_NE(root, index);
  ASSERT_NE(index, root);
}

TEST_F(TransformIndexTest, RootIsLessThanNonRoot) {
  TransformIndex root;
  TransformIndex index{0};

  ASSERT_LT(root, index);
}

TEST_F(TransformIndexTest, RootIsNotLessThanRoot) {
  TransformIndex root;

  ASSERT_FALSE(root < root);
}

TEST_F(TransformIndexTest, SmallerValueAtSameLevelMeansLess) {
  TransformIndex one{1, 2, 3};
  TransformIndex two{1, 3, 3};

  ASSERT_LT(one, two);
}

TEST_F(TransformIndexTest, Ordering) {
  std::vector<TransformIndex> indices{
      TransformIndex{3, 2, 1}, TransformIndex{2, 1, 3, 7}, TransformIndex{1},
      TransformIndex{1, 0},    TransformIndex(),           TransformIndex{2},
  };

  ASSERT_THAT(indices,
              WhenSorted(ElementsAre(TransformIndex(), TransformIndex{1},
                                     TransformIndex{1, 0}, TransformIndex{2},
                                     TransformIndex{2, 1, 3, 7},
                                     TransformIndex{3, 2, 1})));
}

TEST_F(TransformIndexTest, FirstChild) {
  TransformIndex index{1, 2, 3};

  TransformIndex expected{1, 2, 3, 0};
  ASSERT_EQ(expected, index.firstChild());
}

TEST_F(TransformIndexTest, FirstChildOfRoot) {
  TransformIndex root;

  ASSERT_EQ(TransformIndex{0}, root.firstChild());
}

TEST_F(TransformIndexTest, FirstChildCannotExceedDepthLimit) {
  TransformIndex index{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  ASSERT_THROW(index.firstChild(), InvalidTransformIndex);
}

TEST_F(TransformIndexTest, NextSibling) {
  TransformIndex index{1, 2, 3};

  TransformIndex expected{1, 2, 4};
  ASSERT_EQ(expected, index.nextSibling());
}

TEST_F(TransformIndexTest, NextSiblingThrowsForRoot) {
  TransformIndex root;

  ASSERT_THROW(root.nextSibling(), InvalidTransformIndex);
}

TEST_F(TransformIndexTest, Parent) {
  TransformIndex index{1, 2, 3};

  TransformIndex expected{1, 2};
  ASSERT_EQ(expected, index.parent());
}

TEST_F(TransformIndexTest, ParentThrowsForRoot) {
  TransformIndex root;

  ASSERT_THROW(root.parent(), InvalidTransformIndex);
}

}  // namespace chaoskit::core

#pragma GCC diagnostic pop
