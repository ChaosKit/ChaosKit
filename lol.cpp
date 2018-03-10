#include <iostream>
#include <string>

#include "ast/ast.h"
#include "ast/helpers.h"

using namespace chaoskit::ast;

class TestVisitor {
 public:
  void operator()(float number) {
    handle_depth();
    std::cout << number;
  }

  void operator()(const Input &input) {
    handle_depth();
    std::cout << "input.";
    switch (input.type()) {
      case Input::X:
        std::cout << "x";
        break;
      case Input::Y:
        std::cout << "y";
        break;
    }
  }

  void operator()(const Parameter &parameter) {
    handle_depth();
    std::cout << "p[" << parameter.index() << "]";
  }

  void operator()(const UnaryFunction &function) {
    handle_depth();
    std::cout << "unary ";
    switch (function.type()) {
      case UnaryFunction::SIN:
        std::cout << "sin";
        break;
      case UnaryFunction::COS:
        std::cout << "cos";
        break;
      case UnaryFunction::TAN:
        std::cout << "tan";
        break;
      case UnaryFunction::MINUS:
        std::cout << "-";
        break;
      case UnaryFunction::SQRT:
        std::cout << "sqrt";
        break;
      case UnaryFunction::ATAN:
        std::cout << "atan";
        break;
      case UnaryFunction::TRUNC:
        std::cout << "trunc";
        break;
      case UnaryFunction::EXP:
        std::cout << "exp";
        break;
      case UnaryFunction::FLOOR:
        std::cout << "floor";
        break;
      case UnaryFunction::CEIL:
        std::cout << "ceil";
        break;
      case UnaryFunction::SIGNUM:
        std::cout << "signum";
        break;
      case UnaryFunction::ABS:
        std::cout << "abs";
        break;
      case UnaryFunction::NOT:
        std::cout << "not";
        break;
    }

    depth_++;
    chaoskit::ast::apply_visitor(*this, function.argument());
    depth_--;
  }

 private:
  size_t depth_ = 0;

  void handle_depth() {
    std::cout << std::endl << std::string(depth_ * 2, ' ');
  }
};

int main() {
  using namespace chaoskit::ast::helpers;

  chaoskit::ast::Node expression = sin(15.0f);

  TestVisitor visitor;
  chaoskit::ast::apply_visitor(visitor, expression);

  return 0;
}
