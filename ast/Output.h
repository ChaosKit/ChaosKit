#ifndef CHAOSKIT_AST_OUTPUT_H
#define CHAOSKIT_AST_OUTPUT_H

#include <ostream>

namespace chaoskit::ast {

class Output {
 public:
  enum Type : char { X, Y };

  explicit Output(Type type) : type_(type) {}

  [[nodiscard]] Type type() const { return type_; }

  bool operator==(const Output& other) const { return type_ == other.type_; }

 private:
  Type type_;
};

std::ostream& operator<<(std::ostream& stream, const Output& output);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_OUTPUT_H
