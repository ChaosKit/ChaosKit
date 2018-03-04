#ifndef CHAOSKIT_PARAMETER_H
#define CHAOSKIT_PARAMETER_H

#include <cstddef>

namespace chaoskit {
namespace ast {

class Parameter {
 public:
  Parameter(size_t index) : index_(index) {}

  float index() const { return index_; }

 private:
  float index_;
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_PARAMETER_H
