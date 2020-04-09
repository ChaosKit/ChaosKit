#ifndef CHAOSKIT_AST_INPUT_H
#define CHAOSKIT_AST_INPUT_H

#include <ostream>

namespace chaoskit::ast {

class Input {
 public:
  enum Type : char { X, Y, COLOR };

  explicit Input(Type type) : type_(type) {}

  [[nodiscard]] Type type() const { return type_; }

  bool operator==(const Input& other) const { return type_ == other.type_; }

 private:
  Type type_;
};

std::ostream& operator<<(std::ostream& stream, const Input& input);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_INPUT_H
