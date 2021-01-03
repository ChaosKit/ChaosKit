#ifndef CHAOSKIT_LIBRARY_FORMULA_H
#define CHAOSKIT_LIBRARY_FORMULA_H

#include "ast/Formula.h"
#include "ast/StaticAffineTransform.h"
#include "ast/helpers.h"

namespace chaoskit::library {

class Formula {
 public:
  virtual ~Formula() = default;
  virtual ast::Formula source() = 0;
  virtual uint32_t paramCount() { return 0; }
  virtual std::vector<float> exampleParams() {
    return std::vector(paramCount(), 1.f);
  }
  virtual ast::StaticAffineTransform examplePreTransform() {
    return ast::StaticAffineTransform();
  }

 protected:
  ast::helpers::InputHelper input;
  ast::helpers::OutputHelper output;
  ast::helpers::ParameterHelper params;

  class VariableHelper {
   public:
    VariableHelper(Formula* formula, std::string name)
        : formula_(formula), name_(std::move(name)) {}

    VariableHelper& operator=(ast::Expression expression) {
      formula_->variables_.emplace_back(name_, std::move(expression));
      return *this;
    }

   private:
    Formula* formula_;
    std::string name_;
  };

  VariableHelper var(std::string name) {
    return VariableHelper(this, std::move(name));
  }
  [[nodiscard]] ast::Formula withVariables(
      const ast::Expression (&list)[2]) const {
    return ast::Formula(list[0], list[1], variables_);
  }

 private:
  std::vector<ast::VariableDeclaration> variables_;
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULA_H
