#ifndef CHAOSKIT_AST_PARAMETER_H
#define CHAOSKIT_AST_PARAMETER_H

#include <cstddef>

namespace chaoskit::ast {

class Parameter {
 public:
  explicit Parameter(size_t index) : index_(index) {}

  [[nodiscard]] size_t index() const { return index_; }

  bool operator==(const Parameter& other) const {
    return index_ == other.index_;
  }

 private:
  size_t index_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_PARAMETER_H
