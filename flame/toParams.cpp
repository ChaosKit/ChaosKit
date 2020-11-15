#include "toParams.h"

namespace chaoskit::flame {

namespace {

void addParamsForBlend(const BlendBase& blend, core::TransformIndex index,
                       core::TransformParams& params) {
  // We're always nesting.
  index = index.firstChild();

  const auto& pre = blend.pre.values;
  params.insert({index, std::vector<float>(pre.begin(), pre.end())});

  index = index.nextSibling().firstChild();
  for (const auto* formula : blend.formulas) {
    params.insert({index, formula->params});
    index = index.nextSibling();
  }
  index = index.parent().nextSibling();

  const auto& post = blend.post.values;
  params.insert({index, std::vector<float>(post.begin(), post.end())});
}

}  // namespace

core::TransformParams toParams(const System& system) {
  core::TransformParams params;
  core::TransformIndex index;

  if (system.isolatedBlend) {
    addParamsForBlend(*system.isolatedBlend, index, params);
  } else {
    auto blendIndex = index.firstChild();
    for (const auto* blend : system.blends) {
      if (!blend->enabled) continue;
      addParamsForBlend(*blend, blendIndex, params);
      blendIndex = blendIndex.nextSibling();
    }
  }

  return params;
}

stdx::optional<core::TransformParams> toCameraParams(const System& system) {
  if (system.finalBlend && system.finalBlend->enabled) {
    core::TransformParams params;
    addParamsForBlend(*system.finalBlend, core::TransformIndex(), params);
    return params;
  }

  return stdx::nullopt;
}

}  // namespace chaoskit::flame
