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

CHAOSKIT_DEFINE_SIMPLE_RELATION(::chaoskit::flame::Document,
                                ::chaoskit::flame::System,
                                &chaoskit::flame::Document::system);

//          1 +----------+ 1
//      +-----+  System  +-----+
//      |     +----------+     |
//      |                      |
//    * |                      | 1
// +----v----+         +-------v------+
// |  Blend  |         |  FinalBlend  |
// +---------+         +--------------+

CHAOSKIT_DEFINE_RELATION(::chaoskit::flame::System, ::chaoskit::flame::Blend,
                         &::chaoskit::flame::System::blends);
CHAOSKIT_DEFINE_RELATION(::chaoskit::flame::System,
                         ::chaoskit::flame::FinalBlend,
                         &::chaoskit::flame::System::finalBlend);
CHAOSKIT_DEFINE_CHILDREN(::chaoskit::flame::System, ::chaoskit::flame::Blend,
                         ::chaoskit::flame::FinalBlend);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::flame::Blend, ::chaoskit::flame::System);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::flame::FinalBlend,
                        ::chaoskit::flame::System);

// +---------+         +--------------+
// |  Blend  |         |  FinalBlend  |
// +----+----+         +-------+------+
//    1 |                      | 1
//      |                      |
//      |   * +-----------+ *  |
//      +----->  Formula  <----+
//            +-----------+

CHAOSKIT_DEFINE_RELATION(::chaoskit::flame::Blend, ::chaoskit::flame::Formula,
                         &::chaoskit::flame::Blend::formulas)
CHAOSKIT_DEFINE_RELATION(::chaoskit::flame::FinalBlend,
                         ::chaoskit::flame::Formula,
                         &::chaoskit::flame::FinalBlend::formulas);

CHAOSKIT_DEFINE_CHILDREN(::chaoskit::flame::Blend, ::chaoskit::flame::Formula)
CHAOSKIT_DEFINE_CHILDREN(::chaoskit::flame::FinalBlend,
                         ::chaoskit::flame::Formula)
CHAOSKIT_DEFINE_PARENTS(::chaoskit::flame::Formula, ::chaoskit::flame::Blend,
                        ::chaoskit::flame::FinalBlend)

#endif  // CHAOSKIT_CORE_STRUCTURES_HIERARCHY_H
