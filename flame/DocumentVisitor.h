#ifndef CHAOSKIT_FLAME_DOCUMENTVISITOR_H
#define CHAOSKIT_FLAME_DOCUMENTVISITOR_H

#include <ast/System.h>
#include "Blend.h"
#include "Document.h"
#include "Formula.h"
#include "System.h"

namespace chaoskit::flame {

class DocumentVisitor {
 public:
  virtual ~DocumentVisitor() = default;

  virtual void visit(Blend& blend){};
  virtual void visit(Document& document){};
  virtual void visit(FinalBlend& blend){};
  virtual void visit(Formula& formula){};
  virtual void visit(System& system){};
};

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_DOCUMENTVISITOR_H
