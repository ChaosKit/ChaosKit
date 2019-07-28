#ifndef CHAOSKIT_UI_GLOBALSTORE_H
#define CHAOSKIT_UI_GLOBALSTORE_H

#include <QObject>
#include "core/structures/Blend.h"
#include "core/structures/Document.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "state/HierarchicalStore.h"

namespace chaoskit::ui {

class GlobalStore : public QObject {
  Q_OBJECT

  using Store =
      state::HierarchicalStore<core::Blend, core::Document, core::FinalBlend,
                               core::Formula, core::System>;
  Store store_;

 private:
  GlobalStore();

 public:
  GlobalStore(const GlobalStore&) = delete;
  void operator=(const GlobalStore&) = delete;

  static GlobalStore& get();

  Store* operator->() { return &store_; }
  Store& operator*() { return store_; }

 public slots:
  void reset();
  void propagateChanges();

 signals:
  void changed(const Store::Changes&);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_GLOBALSTORE_H
