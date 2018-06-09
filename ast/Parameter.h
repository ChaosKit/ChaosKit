#ifndef CHAOSKIT_PARAMETER_H
#define CHAOSKIT_PARAMETER_H

#include <cstddef>
#include <ostream>

namespace chaoskit {
namespace ast {

class Parameter {
 public:
  Parameter(size_t index) : index_(index) {}

  size_t index() const { return index_; }

  bool operator==(const Parameter& other) const {
    return index_ == other.index_;
  }

 private:
  size_t index_;
};

std::ostream& operator<<(std::ostream& stream, const Parameter& parameter);

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_PARAMETER_H
