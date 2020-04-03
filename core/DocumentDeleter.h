#ifndef CHAOSKIT_CORE_DOCUMENTDELETER_H
#define CHAOSKIT_CORE_DOCUMENTDELETER_H

#include "DocumentVisitor.h"

namespace chaoskit::core {

class DocumentDeleter : public DocumentVisitor {
 public:
  void visit(Blend& blend) override;
  void visit(Document& document) override;
  void visit(FinalBlend& blend) override;
  void visit(System& system) override;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_DOCUMENTDELETER_H
