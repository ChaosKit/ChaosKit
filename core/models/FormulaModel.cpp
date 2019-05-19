#include "FormulaModel.h"

namespace chaoskit::core {

const ast::Formula& FormulaModel::source() const {
  return data_.lock()->source;
}
void FormulaModel::setSource(const ast::Formula& source) {
  data_.lock()->source = source;
}

const std::vector<float>& FormulaModel::params() const {
  return data_.lock()->params;
}
void FormulaModel::setParams(const std::vector<float>& params) {
  data_.lock()->params = params;
}

const Formula::Weight& FormulaModel::weight() const {
  return data_.lock()->weight;
}
void FormulaModel::setWeight(float x, float y) { setWeight({x, y}); }
void FormulaModel::setWeight(float xy) { setWeight({xy, xy}); }
void FormulaModel::setWeight(const Formula::Weight& weight) {
  data_.lock()->weight = weight;
}

}  // namespace chaoskit::core
