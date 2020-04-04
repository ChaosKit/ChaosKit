#include "io.h"
#include <algorithm>
#include <fstream>
#include "chaoskit.pb.h"
#include "library/ColoringMethodType.h"
#include "library/FormulaType.h"

using chaoskit::library::ColoringMethodType;
using chaoskit::library::FormulaType;

namespace chaoskit::io {

namespace {

void readTransform(const Transform& proto, core::Transform* transform) {
  transform->values = {proto.m11(), proto.m21(), proto.m31(),
                       proto.m12(), proto.m22(), proto.m32()};
}

void readFormula(const Formula& proto, core::Formula* formula) {
  auto formulaType = FormulaType::_from_string_nothrow(proto.type().c_str());
  if (!formulaType) {
    throw Error("Unsupported formula type: " + proto.type());
  }

  formula->setType(*formulaType);
  auto& paramsProto = proto.params();
  std::copy(paramsProto.begin(), paramsProto.end(),
            std::back_inserter(formula->params));
  formula->weight.x = proto.weight().x();
  formula->weight.y = proto.weight().y();
}

void readColoringMethod(const ColoringMethod& proto,
                        core::ColoringMethod* coloringMethod) {
  switch (proto.method_case()) {
    case ColoringMethod::METHOD_NOT_SET:
      coloringMethod->setType(ColoringMethodType::Noop);
      break;
    case ColoringMethod::kSingleColor:
      coloringMethod->setType(ColoringMethodType::SingleColor);
      coloringMethod->params.push_back(proto.single_color());
      break;
    case ColoringMethod::kDistance:
      coloringMethod->setType(ColoringMethodType::Distance);
      coloringMethod->params.push_back(proto.distance());
      break;
  }
}

void readBlendBase(const Blend& proto, core::BlendBase* blend) {
  for (const auto& formulaProto : proto.formulas()) {
    auto* formula = new core::Formula;
    readFormula(formulaProto, formula);
    blend->formulas.push_back(formula);
  }
  if (proto.has_pre()) readTransform(proto.pre(), &blend->pre);
  if (proto.has_post()) readTransform(proto.post(), &blend->post);
  readColoringMethod(proto.coloring_method(), &blend->coloringMethod);
  blend->enabled = proto.enabled();
}

void readBlend(const Blend& proto, core::Blend* blend) {
  readBlendBase(proto, blend);
  blend->weight = proto.weight();
}

void readSystem(const System& proto, core::System* system) {
  for (const auto& blendProto : proto.blends()) {
    auto* blend = new core::Blend;
    readBlend(blendProto, blend);
    system->blends.push_back(blend);
  }

  system->finalBlend = new core::FinalBlend;
  readBlendBase(proto.final_blend(), system->finalBlend);
}

void readDocument(const Document& proto, core::Document* document) {
  if (!proto.has_system()) {
    throw Error("System needs to be defined");
  }
  if (proto.width() == 0) {
    throw Error("Document width must be greater than 0");
  }
  if (proto.height() == 0) {
    throw Error("Document height must be greater than 0");
  }

  document->gamma = proto.gamma() == 0.f ? 2.2f : proto.gamma();
  document->exposure = proto.exposure();
  document->vibrancy = proto.vibrancy();
  document->width = proto.width();
  document->height = proto.height();
  // TODO: figure out validation with the registry
  if (proto.has_color_map()) {
    document->colorMap = proto.color_map().name();
  }
  document->system = new core::System;
  readSystem(proto.system(), document->system);
}

void writeTransform(const core::Transform& transform, Transform* proto) {
  proto->set_m11(transform.values[0]);
  proto->set_m21(transform.values[1]);
  proto->set_m31(transform.values[2]);
  proto->set_m12(transform.values[3]);
  proto->set_m22(transform.values[4]);
  proto->set_m32(transform.values[5]);
}

void writeFormula(const core::Formula& formula, Formula* proto) {
  proto->set_type(formula.type._to_string());
  for (float param : formula.params) {
    proto->add_params(param);
  }
  proto->mutable_weight()->set_x(formula.weight.x);
  proto->mutable_weight()->set_y(formula.weight.x);
}

void writeColoringMethod(const core::ColoringMethod& coloringMethod,
                         ColoringMethod* proto) {
  switch (coloringMethod.type) {
    case ColoringMethodType::Noop:
      // Do nothing.
      break;
    case ColoringMethodType::SingleColor:
      proto->set_single_color(coloringMethod.params.at(0));
      break;
    case ColoringMethodType::Distance:
      proto->set_distance(coloringMethod.params.at(0));
      break;
  }
}

void writeBlendBase(const core::BlendBase& blend, Blend* proto) {
  for (const auto* formula : blend.formulas) {
    writeFormula(*formula, proto->add_formulas());
  }
  writeTransform(blend.pre, proto->mutable_pre());
  writeTransform(blend.post, proto->mutable_post());
  writeColoringMethod(blend.coloringMethod, proto->mutable_coloring_method());
  proto->set_enabled(blend.enabled);
}

void writeBlend(const core::Blend& blend, Blend* proto) {
  writeBlendBase(blend, proto);
  proto->set_weight(blend.weight);
}

void writeSystem(const core::System& system, System* proto) {
  for (const auto* blend : system.blends) {
    writeBlend(*blend, proto->add_blends());
  }
  if (system.finalBlend) {
    writeBlendBase(*system.finalBlend, proto->mutable_final_blend());
  }
}

void writeDocument(const core::Document& document, Document* proto) {
  proto->set_gamma(document.gamma);
  proto->set_exposure(document.exposure);
  proto->set_vibrancy(document.vibrancy);
  proto->set_width(document.width);
  proto->set_height(document.height);
  proto->mutable_color_map()->set_name(document.colorMap);
  if (document.system) {
    writeSystem(*document.system, proto->mutable_system());
  }
}

}  // namespace

void loadFromFile(const std::string& path, core::Document* document) {
  std::ifstream stream(path, std::ios::in | std::ios::binary);
  if (stream.fail()) {
    throw Error("Could not read the file");
  }

  Document documentProto;
  if (!documentProto.ParseFromIstream(&stream)) {
    throw Error("Could not parse the file");
  }

  readDocument(documentProto, document);
}

void saveToFile(const std::string& path, const core::Document& document) {
  std::ofstream stream(path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
  if (stream.fail()) {
    throw Error("Could not open the file for writing");
  }

  Document documentProto;
  writeDocument(document, &documentProto);
  if (!documentProto.SerializeToOstream(&stream)) {
    throw Error("Could not write the file");
  }
}

}  // namespace chaoskit::io