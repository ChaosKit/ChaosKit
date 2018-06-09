#include <iostream>
#include <string>

#include "ast/ast.h"
#include "ast/helpers.h"
#include "core/Point.h"
#include "core/SimpleInterpreter.h"

using chaoskit::ast::Formula;
using chaoskit::core::Point;
using chaoskit::core::SimpleInterpreter;
using namespace chaoskit::ast::helpers;

int main() {
  InputHelper input;
  Formula formula{
    input.x() + 1.f,
    input.y() + 1.f
  };
  auto system = make_system(formula);

  SimpleInterpreter interpreter(system);
  interpreter.initialize({0.f, 0.f});
  interpreter.step();
  interpreter.step();
  auto result = interpreter.step();

  std::cout << result << std::endl;

  return 0;
}
