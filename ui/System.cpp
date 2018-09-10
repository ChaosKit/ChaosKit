#include "System.h"
#include <library/DeJong.h>

using chaoskit::core::Blend;
using chaoskit::core::Formula;
using chaoskit::core::Transform;
using chaoskit::library::DeJong;

namespace chaoskit {
namespace ui {

System::System() {
  Formula formula = Formula::build<DeJong>(
      {9.379666578024626e-01f, 1.938709271140397e+00f, -1.580897020176053e-01f,
       -1.430070123635232e+00f});

  Blend blend;
  blend.formulas.push_back(formula);

  internal_system_.blends.push_back(blend);
  internal_system_.final_blend =
      Blend{Transform::identity(),
            Transform::fromScale(.5f, 1.f).translate(0.3f, 0.5f)};
}

}  // namespace ui
}  // namespace chaoskit
