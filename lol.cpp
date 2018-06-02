#include <iostream>
#include <string>

#include "ast/ast.h"
#include "library/DeJong.h"

int main() {
  chaoskit::ast::Node system =
      chaoskit::ast::System({chaoskit::ast::LimitedBlend(
          chaoskit::ast::Blend({chaoskit::ast::WeightedFormula(
              chaoskit::library::DeJong().source())}),
          42.f)});

  chaoskit::ast::apply_visitor(chaoskit::ast::StreamPrinter(std::cout), system);

  return 0;
}
