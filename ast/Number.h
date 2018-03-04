#ifndef CHAOSKIT_NUMBER_H
#define CHAOSKIT_NUMBER_H

namespace chaoskit {
namespace ast {

class Number {
 public:
  Number(float value) : value_(value) {}

  float value() const { return value_; }

 private:
  float value_;
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_NUMBER_H
