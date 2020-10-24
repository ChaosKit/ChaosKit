#ifndef CHAOSKIT_AST_AFFINETRANSFORM_H
#define CHAOSKIT_AST_AFFINETRANSFORM_H

namespace chaoskit::ast {

class AffineTransform {
 public:
  bool operator==(const AffineTransform& other) const { return true; }
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_AFFINETRANSFORM_H
