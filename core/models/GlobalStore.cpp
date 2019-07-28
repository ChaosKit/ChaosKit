#include "GlobalStore.h"

#include "core/structures/Blend.h"
#include "core/structures/Document.h"
#include "core/structures/System.h"

namespace chaoskit::core {

GlobalStore::GlobalStore() : store_() { reset(); }

GlobalStore& GlobalStore::get() {
  static GlobalStore instance;
  return instance;
}

void GlobalStore::reset() {
  store_.clearAll();

  state::Id documentId = store_.create<Document>();
  state::Id systemId =
      store_.associateNewChildWith<Document, System>(documentId);
  store_.associateNewChildWith<System, FinalBlend>(systemId);
}

}  // namespace chaoskit::core
