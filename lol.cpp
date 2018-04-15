#include <iostream>
#include <string>

#include "ast/ast.h"
#include "library/DeJong.h"

int main() {
  chaoskit::ast::Node formula = chaoskit::library::DeJong().source();

  chaoskit::ast::apply_visitor(chaoskit::ast::StreamPrinter(std::cout),
                               formula);

  return 0;
}
