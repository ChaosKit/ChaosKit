#include "Node.h"
#include "util.h"

namespace chaoskit::ast {

Node::Node(const Expression &expression)
    : NodeAlias(static_variant_cast<NodeAlias>(expression)) {}

GENERATE_NODE_TYPE(float)

}  // namespace chaoskit::ast
