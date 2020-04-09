#ifndef CHAOSKIT_CORE_STRUCTURES_HIERARCHY_H
#define CHAOSKIT_CORE_STRUCTURES_HIERARCHY_H

#include "Blend.h"
#include "Document.h"
#include "Formula.h"
#include "System.h"
#include "state/hierarchy.h"

/** Definition of the hierarchy formed by the structures.
 *
 * Here's a UML-ish diagram that represents all definitions below:
 *
 *            +----------+
 *            | Document |
 *            +-----+----+
 *                1 |
 *                  | 1
 *          1 +-----v----+ 1
 *      +-----+  System  +-----+
 *      |     +----------+     |
 *      |                      |
 *    * |                      | 1
 * +----v----+         +-------v------+
 * |  Blend  |         |  FinalBlend  |
 * +----+----+         +-------+------+
 *    1 |                      | 1
 *      |                      |
 *      |   * +-----------+ *  |
 *      +----->  Formula  <----+
 *            +-----------+
 */

// +----------+ 1   1 +----------+
// | Document +------->  System  |
// +----------+       +----------+

CHAOSKIT_DEFINE_SIMPLE_RELATION(::chaoskit::core::Document,
                                ::chaoskit::core::System,
                                &chaoskit::core::Document::system);

//          1 +----------+ 1
//      +-----+  System  +-----+
//      |     +----------+     |
//      |                      |
//    * |                      | 1
// +----v----+         +-------v------+
// |  Blend  |         |  FinalBlend  |
// +---------+         +--------------+

CHAOSKIT_DEFINE_RELATION(::chaoskit::core::System, ::chaoskit::core::Blend,
                         &::chaoskit::core::System::blends);
CHAOSKIT_DEFINE_RELATION(::chaoskit::core::System, ::chaoskit::core::FinalBlend,
                         &::chaoskit::core::System::finalBlend);
CHAOSKIT_DEFINE_CHILDREN(::chaoskit::core::System, ::chaoskit::core::Blend,
                         ::chaoskit::core::FinalBlend);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::core::Blend, ::chaoskit::core::System);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::core::FinalBlend, ::chaoskit::core::System);

// +---------+         +--------------+
// |  Blend  |         |  FinalBlend  |
// +----+----+         +-------+------+
//    1 |                      | 1
//      |                      |
//      |   * +-----------+ *  |
//      +----->  Formula  <----+
//            +-----------+

CHAOSKIT_DEFINE_RELATION(::chaoskit::core::Blend, ::chaoskit::core::Formula,
                         &::chaoskit::core::Blend::formulas);
CHAOSKIT_DEFINE_RELATION(::chaoskit::core::FinalBlend,
                         ::chaoskit::core::Formula,
                         &::chaoskit::core::FinalBlend::formulas);

CHAOSKIT_DEFINE_CHILDREN(::chaoskit::core::Blend, ::chaoskit::core::Formula);
CHAOSKIT_DEFINE_CHILDREN(::chaoskit::core::FinalBlend,
                         ::chaoskit::core::Formula);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::core::Formula, ::chaoskit::core::Blend,
                        ::chaoskit::core::FinalBlend);

#endif  // CHAOSKIT_CORE_STRUCTURES_HIERARCHY_H
