#ifndef CHAOSKIT_CORE_MODELS_GLOBALSTORE_H
#define CHAOSKIT_CORE_MODELS_GLOBALSTORE_H

#include <memory>
#include "core/structures/Blend.h"
#include "core/structures/Document.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "state/HierarchicalStore.h"
#include "state/Id.h"

namespace chaoskit::core {

class GlobalStore {
  using Store =
      state::HierarchicalStore<Document, System, Blend, FinalBlend, Formula>;
  Store store_;

 public:
  GlobalStore(const GlobalStore&) = delete;
  void operator=(const GlobalStore&) = delete;

  static GlobalStore& get();

  void reset();

  [[nodiscard]] const Document* document() const {
    return store_.find<Document>(documentId());
  }
  [[nodiscard]] const System* system() const {
    return store_.find<System>(systemId());
  }

  [[nodiscard]] state::Id documentId() const {
    return store_.lastId<Document>();
  }
  [[nodiscard]] state::Id systemId() const { return store_.lastId<System>(); }

  Store* operator->() { return &store_; }
  Store& operator*() { return store_; }

 private:
  GlobalStore();
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MODELS_GLOBALSTORE_H
