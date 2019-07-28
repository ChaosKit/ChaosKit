#include "GlobalStore.h"

namespace chaoskit::ui {

GlobalStore::GlobalStore() : store_() {
  store_.setTrackingChanges(true);
  reset();
}

GlobalStore& GlobalStore::get() {
  static GlobalStore instance;
  return instance;
}

void GlobalStore::reset() {
  store_.clearAll();

  state::Id documentId = store_.create<core::Document>();
  state::Id systemId =
      store_.associateNewChildWith<core::Document, core::System>(documentId);
  store_.associateNewChildWith<core::System, core::FinalBlend>(systemId);
}

void GlobalStore::propagateChanges() {
  if (store_.changes().empty()) return;

  emit(changed(store_.changes()));
  store_.resetChanges();
}

}  // namespace chaoskit::ui
