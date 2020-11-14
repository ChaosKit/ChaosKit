#ifndef CHAOSKIT_UI_DOCUMENTSTORE_H
#define CHAOSKIT_UI_DOCUMENTSTORE_H

#include "flame/Blend.h"
#include "flame/Document.h"
#include "flame/Formula.h"
#include "flame/System.h"
#include "flame/hierarchy.h"
#include "state/HierarchicalStore.h"

namespace chaoskit::ui {

using DocumentStore =
    state::HierarchicalStore<flame::Blend, flame::Document, flame::FinalBlend,
                             flame::Formula, flame::System>;

}

#endif  // CHAOSKIT_UI_DOCUMENTSTORE_H
