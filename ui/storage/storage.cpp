#include "storage.h"
#include <algorithm>
#include <fstream>
#include <magic_enum.hpp>
#include "chaoskit.pb.h"
#include "library/ColoringMethodType.h"
#include "library/FormulaType.h"

using chaoskit::library::ColoringMethodType;
using chaoskit::library::FormulaType;

namespace chaoskit::ui {

namespace {

void readTransform(const Transform& proto, flame::Transform* transform) {
  transform->values = {proto.m11(), proto.m21(), proto.m31(),
                       proto.m12(), proto.m22(), proto.m32()};
}

void readFormula(const Formula& proto, flame::Formula* formula) {
  auto formulaType = magic_enum::enum_cast<FormulaType>(proto.type());
  if (!formulaType) {
    throw StorageError("Unsupported formula type: " + proto.type());
  }

  formula->setType(*formulaType);
  auto& paramsProto = proto.params();
  std::copy(paramsProto.begin(), paramsProto.end(),
            std::back_inserter(formula->params));
  formula->weight.x = proto.weight().x();
  formula->weight.y = proto.weight().y();
}

void readColoringMethod(const ColoringMethod& proto,
                        flame::ColoringMethod* coloringMethod) {
  switch (proto.method_case()) {
    case ColoringMethod::METHOD_NOT_SET:
      coloringMethod->setType(ColoringMethodType::Noop);
      break;
    case ColoringMethod::kSingleColor:
      coloringMethod->setType(ColoringMethodType::SingleColor);
      coloringMethod->params = {proto.single_color()};
      break;
    case ColoringMethod::kDistance:
      coloringMethod->setType(ColoringMethodType::Distance);
      coloringMethod->params = {proto.distance()};
      break;
  }
}

void readBlendBase(const Blend& proto, flame::BlendBase* blend) {
  for (const auto& formulaProto : proto.formulas()) {
    auto* formula = new flame::Formula;
    readFormula(formulaProto, formula);
    blend->formulas.push_back(formula);
  }
  if (proto.has_pre()) readTransform(proto.pre(), &blend->pre);
  if (proto.has_post()) readTransform(proto.post(), &blend->post);
  readColoringMethod(proto.coloring_method(), &blend->coloringMethod);
  blend->enabled = proto.enabled();
}

void readBlend(const Blend& proto, flame::Blend* blend) {
  readBlendBase(proto, blend);
  blend->name = proto.name();
  blend->weight = proto.weight();
}

void readSystem(const System& proto, flame::System* system) {
  for (const auto& blendProto : proto.blends()) {
    auto* blend = new flame::Blend;
    readBlend(blendProto, blend);
    system->blends.push_back(blend);
  }

  system->finalBlend = new flame::FinalBlend;
  readBlendBase(proto.final_blend(), system->finalBlend);

  if (proto.ttl() != 0) {
    system->ttl = proto.ttl();
  }
  system->skip = proto.skip();

  if (proto.has_initial_transform()) {
    readTransform(proto.initial_transform(), &system->initialTransform);
  }

  if (proto.has_isolated_blend()) {
    system->isolatedBlend = system->blends.at(proto.isolated_blend().index());
  }
}

void readDocument(const Document& proto, flame::Document* document) {
  if (!proto.has_system()) {
    throw StorageError("System needs to be defined");
  }
  if (proto.width() == 0) {
    throw StorageError("Document width must be greater than 0");
  }
  if (proto.height() == 0) {
    throw StorageError("Document height must be greater than 0");
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
  document->system = new flame::System;
  readSystem(proto.system(), document->system);
}

void writeTransform(const flame::Transform& transform, Transform* proto) {
  proto->set_m11(transform.values[0]);
  proto->set_m21(transform.values[1]);
  proto->set_m31(transform.values[2]);
  proto->set_m12(transform.values[3]);
  proto->set_m22(transform.values[4]);
  proto->set_m32(transform.values[5]);
}

void writeFormula(const flame::Formula& formula, Formula* proto) {
  proto->set_type(
      std::string(magic_enum::enum_name<FormulaType>(formula.type)));
  for (float param : formula.params) {
    proto->add_params(param);
  }
  proto->mutable_weight()->set_x(formula.weight.x);
  proto->mutable_weight()->set_y(formula.weight.x);
}

void writeColoringMethod(const flame::ColoringMethod& coloringMethod,
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

void writeBlendBase(const flame::BlendBase& blend, Blend* proto) {
  for (const auto* formula : blend.formulas) {
    writeFormula(*formula, proto->add_formulas());
  }
  writeTransform(blend.pre, proto->mutable_pre());
  writeTransform(blend.post, proto->mutable_post());
  writeColoringMethod(blend.coloringMethod, proto->mutable_coloring_method());
  proto->set_enabled(blend.enabled);
}

void writeBlend(const flame::Blend& blend, Blend* proto) {
  writeBlendBase(blend, proto);
  proto->set_name(blend.name);
  proto->set_weight(blend.weight);
}

void writeSystem(const flame::System& system, System* proto) {
  for (const auto* blend : system.blends) {
    writeBlend(*blend, proto->add_blends());
  }
  if (system.finalBlend) {
    writeBlendBase(*system.finalBlend, proto->mutable_final_blend());
  }
  proto->set_ttl(system.ttl);
  proto->set_skip(system.skip);
  writeTransform(system.initialTransform, proto->mutable_initial_transform());

  if (system.isolatedBlend) {
    const auto& blends = system.blends;
    auto it = std::find(blends.begin(), blends.end(), system.isolatedBlend);
    if (it != blends.end()) {
      proto->mutable_isolated_blend()->set_index(
          std::distance(blends.begin(), it));
    }
  }
}

void writeDocument(const flame::Document& document, Document* proto) {
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

void loadFromFile(const std::string& path, flame::Document* document) {
  std::ifstream stream(path, std::ios::in | std::ios::binary);
  if (stream.fail()) {
    throw StorageError("Could not read the file");
  }

  Document documentProto;
  if (!documentProto.ParseFromIstream(&stream)) {
    throw StorageError("Could not parse the file");
  }

  readDocument(documentProto, document);
}

void saveToFile(const std::string& path, const flame::Document& document) {
  std::ofstream stream(path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
  if (stream.fail()) {
    throw StorageError("Could not open the file for writing");
  }

  Document documentProto;
  writeDocument(document, &documentProto);
  if (!documentProto.SerializeToOstream(&stream)) {
    throw StorageError("Could not write the file");
  }
}

}  // namespace chaoskit::ui
