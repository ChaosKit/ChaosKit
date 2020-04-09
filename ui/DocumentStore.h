#ifndef CHAOSKIT_UI_DOCUMENTSTORE_H
#define CHAOSKIT_UI_DOCUMENTSTORE_H

#include "core/structures/Blend.h"
#include "core/structures/Document.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "core/structures/hierarchy.h"
#include "state/HierarchicalStore.h"

namespace chaoskit::ui {

using DocumentStore =
    state::HierarchicalStore<core::Blend, core::Document, core::FinalBlend,
                             core::Formula, core::System>;

}

#endif  // CHAOSKIT_UI_DOCUMENTSTORE_H
