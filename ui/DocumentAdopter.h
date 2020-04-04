#ifndef CHAOSKIT_UI_DOCUMENTADOPTER_H
#define CHAOSKIT_UI_DOCUMENTADOPTER_H

#include "DocumentStore.h"
#include "core/DocumentVisitor.h"
#include "state/Id.h"

namespace chaoskit::ui {

class DocumentAdopter : public core::DocumentVisitor {
 public:
  DocumentAdopter() = delete;
  explicit DocumentAdopter(DocumentStore& store) : store_(store), idStack_() {}

  void visit(core::Blend& blend) override;
  void visit(core::Document& document) override;
  void visit(core::FinalBlend& blend) override;
  void visit(core::Formula& formula) override;
  void visit(core::System& system) override;

 private:
  DocumentStore& store_;
  std::vector<state::Id> idStack_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_DOCUMENTADOPTER_H
