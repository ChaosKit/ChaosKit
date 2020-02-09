#ifndef CHAOSKIT_AST_OUTPUT_H
#define CHAOSKIT_AST_OUTPUT_H

#include "thirdparty/better_enums/enum.h"

namespace chaoskit::ast {

BETTER_ENUM(Output_Type, char, X, Y)

class Output {
 public:
  typedef Output_Type Type;

  explicit Output(Type type) : type_(type) {}

  [[nodiscard]] Type type() const { return type_; }

  bool operator==(const Output& other) const { return type_ == other.type_; }

 private:
  Type type_;
};

std::ostream& operator<<(std::ostream& stream, const Output& output);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_OUTPUT_H
