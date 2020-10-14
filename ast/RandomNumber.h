#ifndef CHAOSKIT_AST_RANDOMNUMBER_H
#define CHAOSKIT_AST_RANDOMNUMBER_H

#include <ostream>

namespace chaoskit::ast {

class RandomNumber {
 public:
  bool operator==(const RandomNumber& other) const { return true; }
};

std::ostream& operator<<(std::ostream& stream, const RandomNumber& random);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_RANDOMNUMBER_H
