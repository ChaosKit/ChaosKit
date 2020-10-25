#ifndef CHAOSKIT_AST_RANDOMNUMBER_H
#define CHAOSKIT_AST_RANDOMNUMBER_H

namespace chaoskit::ast {

class RandomNumber {
 public:
  bool operator==(const RandomNumber& other) const { return true; }
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_RANDOMNUMBER_H
