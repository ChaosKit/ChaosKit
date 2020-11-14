#ifndef CHAOSKIT_UI_DOCUMENTADOPTER_H
#define CHAOSKIT_UI_DOCUMENTADOPTER_H

#include "DocumentStore.h"
#include "flame/DocumentVisitor.h"
#include "state/Id.h"

namespace chaoskit::ui {

class DocumentAdopter : public flame::DocumentVisitor {
 public:
  DocumentAdopter() = delete;
  explicit DocumentAdopter(DocumentStore& store) : store_(store), idStack_() {}

  void visit(flame::Blend& blend) override;
  void visit(flame::Document& document) override;
  void visit(flame::FinalBlend& blend) override;
  void visit(flame::Formula& formula) override;
  void visit(flame::System& system) override;

 private:
  DocumentStore& store_;
  std::vector<state::Id> idStack_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_DOCUMENTADOPTER_H
