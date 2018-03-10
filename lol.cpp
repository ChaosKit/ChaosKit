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

  void operator()(const BinaryFunction& function) {
    handle_depth();
    std::cout << "binary ";
    switch (function.type()) {
      case BinaryFunction::ADD:
        std::cout << "+";
        break;
      case BinaryFunction::SUBTRACT:
        std::cout << "-";
        break;
      case BinaryFunction::MULTIPLY:
        std::cout << "*";
        break;
      case BinaryFunction::DIVIDE:
        std::cout << "/";
        break;
      case BinaryFunction::POWER:
        std::cout << "pow";
        break;
      case BinaryFunction::MODULO:
        std::cout << "mod";
        break;
      case BinaryFunction::AND:
        std::cout << "&&";
        break;
      case BinaryFunction::OR:
        std::cout << "||";
        break;
      case BinaryFunction::LESS_THAN:
        std::cout << "<";
        break;
      case BinaryFunction::GREATER_THAN:
        std::cout << ">";
        break;
      case BinaryFunction::EQUALS:
        std::cout << "==";
        break;
      case BinaryFunction::LESS_THAN_OR_EQUAL:
        std::cout << "<=";
        break;
      case BinaryFunction::GREATER_THAN_OR_EQUAL:
        std::cout << ">=";
        break;
      case BinaryFunction::DISTANCE:
        std::cout << "distance";
        break;
    }

    depth_++;
    chaoskit::ast::apply_visitor(*this, function.first());
    chaoskit::ast::apply_visitor(*this, function.second());
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

  chaoskit::ast::Node expression = pow(sin(2.f) / cos(3.f), 2.f);

  TestVisitor visitor;
  chaoskit::ast::apply_visitor(visitor, expression);

  return 0;
}
