#ifndef CHAOSKIT_AST_VARIABLENAME_H
#define CHAOSKIT_AST_VARIABLENAME_H

#include <string>
#include <utility>

namespace chaoskit::ast {

class VariableName {
 public:
  explicit VariableName(std::string name) : name_(std::move(name)) {}

  // Explicitly allow casting strings to variable names.
  // NOLINTNEXTLINE(hicpp-explicit-conversions, google-explicit-constructor)
  VariableName(const char* name) : name_(name) {}

  [[nodiscard]] const std::string& name() const { return name_; }

  bool operator==(const VariableName& other) const {
    return other.name_ == name_;
  }

 private:
  std::string name_;
};

std::ostream& operator<<(std::ostream& stream, const VariableName& name);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_VARIABLENAME_H
