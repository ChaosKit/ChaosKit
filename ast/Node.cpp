#include "Node.h"
#include "util.h"

namespace chaoskit {
namespace ast {

Node::Node(const Expression &expression)
    : NodeAlias(static_variant_cast<NodeAlias>(expression)) {}

GENERATE_NODE_TYPE(float)

}  // namespace ast
}  // namespace chaoskit
