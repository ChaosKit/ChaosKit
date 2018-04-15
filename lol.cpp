#include <iostream>
#include <string>

#include "ast/ast.h"
#include "ast/helpers.h"

int main() {
  using namespace chaoskit::ast::helpers;

  chaoskit::ast::Node formula =
      chaoskit::ast::Formula(pow(sin(2.f) / cos(3.f), 2.f), n(3.f) - atan(2.f));

  chaoskit::ast::apply_visitor(chaoskit::ast::StreamPrinter(std::cout),
                               formula);

  return 0;
}
