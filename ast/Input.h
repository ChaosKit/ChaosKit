#ifndef CHAOSKIT_AST_INPUT_H
#define CHAOSKIT_AST_INPUT_H

#include <enum.h>
#include <ostream>

namespace chaoskit::ast {

BETTER_ENUM(Input_Type, char, X, Y)

class Input {
 public:
  typedef Input_Type Type;

  Input(Type type) : type_(type) {}

  Type type() const { return type_; }

  bool operator==(const Input& other) const { return type_ == other.type_; }

 private:
  Type type_;
};

std::ostream& operator<<(std::ostream& stream, const Input& input);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_INPUT_H
