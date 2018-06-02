#include <iostream>
#include <string>

#include "ast/ast.h"
#include "library/DeJong.h"

int main() {
  chaoskit::ast::Node blend = chaoskit::ast::Blend({
     chaoskit::ast::WeightedFormula(
         chaoskit::library::DeJong().source()
     )
  });

  chaoskit::ast::apply_visitor(chaoskit::ast::StreamPrinter(std::cout),
                               blend);

  return 0;
}
