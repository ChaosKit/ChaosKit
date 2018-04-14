#ifndef CHAOSKIT_AST_UTIL_H
#define CHAOSKIT_AST_UTIL_H

#include <mapbox/variant.hpp>
#include <string>

namespace chaoskit {
namespace ast {

using mapbox::util::apply_visitor;

class StreamPrinter {
  std::ostream& stream_;

 public:
  explicit StreamPrinter(std::ostream& stream) : stream_(stream) {}

  template <typename T>
  void operator()(const T& node) const {
    stream_ << node;
  }
};

template <typename T>
std::string node_type() {
  return "<unknown node type>";
}
template <typename T>
std::string node_type(const T& node) {
  return node_type<T>();
}

#define GENERATE_NODE_TYPE(type)  \
  template <>                     \
  std::string node_type<type>() { \
    return #type;                 \
  }

GENERATE_NODE_TYPE(float)

class NodeTypeVisitor {
 public:
  template <typename T>
  std::string operator()(const T& node) {
    return node_type(node);
  }
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_AST_UTIL_H
