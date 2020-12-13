#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <gmock/gmock.h>
#include <magic_enum.hpp>
#include "ast.h"
#include "helpers.h"

namespace chaoskit::ast {

template <>
std::string node_type(const Node& node) {
  return apply_visitor(NodeTypeVisitor{}, node);
}

std::ostream& operator<<(std::ostream& stream, const Node& node) {
  apply_visitor(StreamPrinter(stream), node);
  return stream;
}

class TreeEqualityVisitor {
  std::vector<const Node*> stack_;
  std::ostream* output_;

 public:
  TreeEqualityVisitor(const Node& expected, std::ostream* output)
      : stack_{&expected}, output_(output) {}

  template <typename T>
  bool operator()(const T& something) {
    const Node* expected = stack_.back();

    if (!expected->is<T>()) {
      return failWithHint(*expected, something);
    }

    const auto& expectedValue = expected->get<T>();
    if (something == expectedValue) {
      return true;
    } else {
      return failWithHint(expectedValue, something);
    }
  }

  bool operator()(const UnaryFunction& function) {
    const Node* expected = stack_.back();

    if (!expected->is<UnaryFunction>()) {
      return failWithHint(*expected, function);
    }

    const auto& expectedValue = expected->get<UnaryFunction>();
    if (expectedValue.type() != function.type()) {
      return failWithHint(expectedValue, function);
    }

    Node expectedArgument = expectedValue.argument();
    stack_.push_back(&expectedArgument);
    bool result = apply_visitor(*this, function.argument());
    stack_.pop_back();
    return result;
  }

  bool operator()(const BinaryFunction& function) {
    const Node* expected = stack_.back();

    if (!expected->is<BinaryFunction>()) {
      return failWithHint(expected, function);
    }

    const auto& expectedValue = expected->get<BinaryFunction>();
    if (expectedValue.type() != function.type()) {
      return failWithHint(expectedValue, function);
    }

    Node expectedFirst = expectedValue.first();
    stack_.push_back(&expectedFirst);
    bool first = apply_visitor(*this, function.first());
    stack_.pop_back();
    if (!first) {
      return first;
    }

    Node expectedSecond = expectedValue.second();
    stack_.push_back(&expectedSecond);
    bool second = apply_visitor(*this, function.second());
    stack_.pop_back();
    return second;
  }

  bool operator()(const Formula& formula) {
    const Node* expected = stack_.back();

    if (!expected->is<Formula>()) {
      return failWithHint(*expected, formula);
    }

    const auto& expectedValue = expected->get<Formula>();

    Node expectedX = expectedValue.x();
    stack_.push_back(&expectedX);
    bool x = apply_visitor(*this, formula.x());
    stack_.pop_back();
    if (!x) {
      return x;
    }

    Node expectedY = expectedValue.y();
    stack_.push_back(&expectedY);
    bool y = apply_visitor(*this, formula.y());
    stack_.pop_back();
    return y;
  }

  // TODO: finish all the other AST nodes when I'm less lazy
  //  bool operator()(const Blend &blend) {
  //    const Node* expected = stack_.back();
  //
  //    if (!expected->is<Blend>()) {
  //      return failWithHint(*expected, blend);
  //    }
  //
  //    const auto& expectedValue = expected->get<Blend>();
  //
  //    Node expectedPre = expectedValue.pre();
  //    stack_.push_back(&expectedPre);
  //    bool pre = apply_visitor(*this, blend.pre());
  //    stack_.pop_back();
  //    if (!pre) {
  //      return pre;
  //    }
  //
  //    Node expectedPost = expectedValue.post();
  //    stack_.push_back(&expectedPost);
  //    bool post = apply_visitor(*this, blend.post());
  //    stack_.pop_back();
  //    if (!post) {
  //      return post;
  //    }
  //  }

 private:
  template <typename T, typename U>
  bool failWithHint(const T& expected, const U& actual) {
    if (output_ != nullptr && stack_.size() > 1) {
      *output_ << "hint: there's " << actual << " instead of " << expected;
    }

    return false;
  }
};

class TreeEqualityMatcher : public testing::MatcherInterface<Node> {
  const Node& expected_;

 public:
  explicit TreeEqualityMatcher(const Node& expected) : expected_(expected) {}

  bool MatchAndExplain(Node tree,
                       testing::MatchResultListener* listener) const override {
    TreeEqualityVisitor visitor(expected_, listener->stream());
    return apply_visitor(visitor, tree);
  }

  void DescribeTo(std::ostream* os) const override {
    apply_visitor(StreamPrinter(*os), expected_);
  }
};

inline testing::Matcher<Node> EqualsTree(const Node& tree) {
  return testing::MakeMatcher(new TreeEqualityMatcher(tree));
}

using testing::ContainerEq;
using testing::Eq;
using testing::Not;

class AstTest : public ::testing::Test {};

TEST_F(AstTest, N) {
  Node result = helpers::n(2.f);
  Node expected = 2.f;

  ASSERT_THAT(result, EqualsTree(expected));
}

TEST_F(AstTest, Input) {
  helpers::InputHelper input;

  Node result = input.x();
  Node expected = Input(Input::Type::X);

  ASSERT_THAT(result, EqualsTree(expected));
}

TEST_F(AstTest, InputXIsNotY) {
  helpers::InputHelper input;

  Node x = input.x();
  Node y = input.y();

  ASSERT_THAT(x, Not(EqualsTree(y)));
}

TEST_F(AstTest, Output) {
  helpers::OutputHelper output;

  Node result = output.y();
  Node expected = Output(Output::Type::Y);

  ASSERT_THAT(result, EqualsTree(expected));
}

TEST_F(AstTest, OutputIsNotInput) {
  helpers::OutputHelper output;
  helpers::InputHelper input;

  Node outputX = output.x();
  Node inputX = input.x();

  ASSERT_THAT(outputX, Not(EqualsTree(inputX)));
}

TEST_F(AstTest, Parameter) {
  helpers::ParameterHelper parameter;

  Node result = parameter[42];
  Node expected = Parameter(42);

  ASSERT_THAT(result, EqualsTree(expected));
}

TEST_F(AstTest, ParameterIndicesMatter) {
  helpers::ParameterHelper parameter;

  Node one = parameter[1];
  Node two = parameter[2];

  ASSERT_THAT(one, Not(EqualsTree(two)));
}

TEST_F(AstTest, UnaryMinusOverload) {
  using namespace helpers;

  Node actual = -helpers::n(2.f);
  Node expected = UnaryFunction(UnaryFunction::Type::MINUS, 2.f);

  ASSERT_THAT(actual, EqualsTree(expected));
}

TEST_F(AstTest, BinaryMinusOverload) {
  using namespace helpers;

  Node actual = n(2.f) - 3.f;
  Node expected = BinaryFunction(BinaryFunction::Type::SUBTRACT, 2.f, 3.f);

  ASSERT_THAT(actual, EqualsTree(expected));
}

}  // namespace chaoskit::ast
