#ifndef CHAOSKIT_CORE_DOCUMENTDELETER_H
#define CHAOSKIT_CORE_DOCUMENTDELETER_H

#include <ast/System.h>
#include "Blend.h"
#include "Document.h"
#include "DocumentVisitor.h"
#include "System.h"

namespace chaoskit::flame {

class DocumentDeleter : public DocumentVisitor {
 public:
  void visit(Blend& blend) override;
  void visit(Document& document) override;
  void visit(FinalBlend& blend) override;
  void visit(System& system) override;
};

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_CORE_DOCUMENTDELETER_H
