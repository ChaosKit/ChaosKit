#ifndef CHAOSKIT_PARAMETER_H
#define CHAOSKIT_PARAMETER_H

#include <cstddef>
#include "util.h"

namespace chaoskit {
namespace ast {

class Parameter {
 public:
  Parameter(size_t index) : index_(index) {}

  float index() const { return index_; }

  bool operator==(const Parameter& other) const {
    return index_ == other.index_;
  }

 private:
  float index_;
};

GENERATE_NODE_TYPE(Parameter)

std::ostream& operator<<(std::ostream& stream, const Parameter& parameter) {
  return stream << node_type(parameter) << "(" << parameter.index() << ")";
}

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_PARAMETER_H
