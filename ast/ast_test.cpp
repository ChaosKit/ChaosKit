#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <gmock/gmock.h>
#include "ast.h"
#include "helpers.h"

namespace chaoskit {
namespace ast {

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

TEST(AstTest, N) {
  Node result = helpers::n(2.f);
  Node expected = 2.f;

  ASSERT_THAT(result, EqualsTree(expected));
}

TEST(AstTest, Input) {
  helpers::InputHelper input;

  Node result = input.x();
  Node expected = Input(Input_Type::X);

  ASSERT_THAT(result, EqualsTree(expected));
}

TEST(AstTest, InputXIsNotY) {
  helpers::InputHelper input;

  Node x = input.x();
  Node y = input.y();

  ASSERT_THAT(x, Not(EqualsTree(y)));
}

TEST(AstTest, Parameter) {
  helpers::ParameterHelper parameter;

  Node result = parameter[42];
  Node expected = Parameter(42);

  ASSERT_THAT(result, EqualsTree(expected));
}

TEST(AstTest, ParameterIndicesMatter) {
  helpers::ParameterHelper parameter;

  Node one = parameter[1];
  Node two = parameter[2];

  ASSERT_THAT(one, Not(EqualsTree(two)));
}

TEST(AstTest, AllUnaryFunctionTypesHaveWorkingHelpers) {
  std::unordered_map<UnaryFunction_Type::_enumerated,
                     std::function<UnaryFunction(const Expression&)>>
      helper_map{{UnaryFunction_Type::SIN, helpers::sin},
                 {UnaryFunction_Type::COS, helpers::cos},
                 {UnaryFunction_Type::ATAN, helpers::atan},
                 {UnaryFunction_Type::CEIL, helpers::ceil},
                 {UnaryFunction_Type::TAN, helpers::tan},
                 {UnaryFunction_Type::MINUS, helpers::negative},
                 {UnaryFunction_Type::SQRT, helpers::sqrt},
                 {UnaryFunction_Type::TRUNC, helpers::trunc},
                 {UnaryFunction_Type::EXP, helpers::exp},
                 {UnaryFunction_Type::FLOOR, helpers::floor},
                 {UnaryFunction_Type::SIGNUM, helpers::signum},
                 {UnaryFunction_Type::ABS, helpers::abs},
                 {UnaryFunction_Type::NOT, helpers::operator!} };

  // Fill sets with type names as strings
  std::unordered_set<std::string> all_types;
  std::unordered_set<std::string> defined_helpers;

  auto names = UnaryFunction::Type::_names();
  std::transform(names.begin(), names.end(),
                 std::inserter(all_types, all_types.end()),
                 [](const char* str) { return std::string(str); });
  std::transform(helper_map.begin(), helper_map.end(),
                 std::inserter(defined_helpers, defined_helpers.end()),
                 [](const decltype(helper_map)::value_type& pair) {
                   return std::string((+pair.first)._to_string());
                 });

  EXPECT_THAT(defined_helpers, ContainerEq(all_types));

  Expression expr = 42.f;
  for (const decltype(helper_map)::value_type& pair : helper_map) {
    auto helper_result = pair.second(expr);
    ASSERT_THAT(helper_result, EqualsTree(UnaryFunction(pair.first, expr)));
  }
}

TEST(AstTest, AllBinaryFunctionTypesHaveWorkingHelpers) {
  std::unordered_map<
      BinaryFunction_Type::_enumerated,
      std::function<BinaryFunction(const Expression&, const Expression&)>>
      helper_map{{BinaryFunction_Type::ADD, (helpers::operator+)},
                 {BinaryFunction_Type::SUBTRACT, helpers::subtract},
                 {BinaryFunction_Type::MULTIPLY, (helpers::operator*)},
                 {BinaryFunction_Type::DIVIDE, (helpers::operator/)},
                 {BinaryFunction_Type::POWER, helpers::pow},
                 {BinaryFunction_Type::MODULO, (helpers::operator%)},
                 {BinaryFunction_Type::AND, (helpers::operator&&)},
                 {BinaryFunction_Type::OR, (helpers::operator||)},
                 {BinaryFunction_Type::LESS_THAN, helpers::lt},
                 {BinaryFunction_Type::GREATER_THAN, helpers::gt},
                 {BinaryFunction_Type::EQUALS, helpers::eq},
                 {BinaryFunction_Type::LESS_THAN_OR_EQUAL, helpers::lte},
                 {BinaryFunction_Type::GREATER_THAN_OR_EQUAL, helpers::gte},
                 {BinaryFunction_Type::DISTANCE, helpers::distance}};

  // Fill sets with type names as strings
  std::unordered_set<std::string> all_types;
  std::unordered_set<std::string> defined_helpers;

  auto names = BinaryFunction::Type::_names();
  std::transform(names.begin(), names.end(),
                 std::inserter(all_types, all_types.end()),
                 [](const char* str) { return std::string(str); });
  std::transform(helper_map.begin(), helper_map.end(),
                 std::inserter(defined_helpers, defined_helpers.end()),
                 [](const decltype(helper_map)::value_type& pair) {
                   return std::string((+pair.first)._to_string());
                 });

  EXPECT_THAT(defined_helpers, ContainerEq(all_types));

  Expression expr = 42.f;
  for (const decltype(helper_map)::value_type& pair : helper_map) {
    auto helper_result = pair.second(expr, expr);
    ASSERT_THAT(helper_result,
                EqualsTree(BinaryFunction(pair.first, expr, expr)));
  }
}

TEST(AstTest, UnaryMinusOverload) {
  using namespace helpers;

  Node actual = -helpers::n(2.f);
  Node expected = UnaryFunction(UnaryFunction_Type::MINUS, 2.f);

  ASSERT_THAT(actual, EqualsTree(expected));
}

TEST(AstTest, BinaryMinusOverload) {
  using namespace helpers;

  Node actual = n(2.f) - 3.f;
  Node expected = BinaryFunction(BinaryFunction_Type::SUBTRACT, 2.f, 3.f);

  ASSERT_THAT(actual, EqualsTree(expected));
}

}  // namespace ast
}  // namespace chaoskit
