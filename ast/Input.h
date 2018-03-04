#ifndef CHAOSKIT_INPUT_H
#define CHAOSKIT_INPUT_H

namespace chaoskit {
namespace ast {

class Input {
 public:
  enum Type { X, Y };

  Input(Type type) : type_(type) {}

  Type type() const { return type_; }

 private:
  Type type_;
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_INPUT_H
