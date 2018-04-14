#ifndef CHAOSKIT_INPUT_H
#define CHAOSKIT_INPUT_H

#include <enum.h>

namespace chaoskit {
namespace ast {

BETTER_ENUM(Input_Type, char, X, Y)

class Input {
 public:
  typedef Input_Type Type;

  Input(Type type) : type_(type) {}

  Type type() const { return type_; }

 private:
  Type type_;
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_INPUT_H
