#ifndef CHAOSKIT_CORE_DOCUMENTVISITOR_H
#define CHAOSKIT_CORE_DOCUMENTVISITOR_H

#include "structures/Document.h"

namespace chaoskit::core {

class DocumentVisitor {
 public:
  virtual ~DocumentVisitor() = default;

  virtual void visit(Blend& blend){};
  virtual void visit(Document& document){};
  virtual void visit(FinalBlend& blend){};
  virtual void visit(Formula& formula){};
  virtual void visit(System& system){};
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_DOCUMENTVISITOR_H
