#ifndef CHAOSKIT_AST_VARIABLEDECLARATION_H
#define CHAOSKIT_AST_VARIABLEDECLARATION_H

#include <ostream>
#include <string>
#include "Expression.h"

namespace chaoskit::ast {

class VariableDeclaration {
 public:
  VariableDeclaration(std::string name, Expression definition)
      : name_(std::move(name)), definition_(std::move(definition)) {}

  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] const Expression& definition() const { return definition_; }

  bool operator==(const VariableDeclaration& other) const {
    return other.name_ == name_ && other.definition_ == definition_;
  }

 private:
  std::string name_;
  Expression definition_;
};

std::ostream& operator<<(std::ostream& stream,
                         const VariableDeclaration& declaration);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_VARIABLEDECLARATION_H
