#ifndef CHAOSKIT_UI_RANDOMIZATIONSETTINGS_H
#define CHAOSKIT_UI_RANDOMIZATIONSETTINGS_H

#include <QVector>
#include "library/ColoringMethodType.h"
#include "library/FormulaType.h"

namespace chaoskit::ui {

class RandomizationSettings {
  int minBlends_ = 1;
  int maxBlends_ = 1;
  int minFormulasInBlend_ = 1;
  int maxFormulasInBlend_ = 1;
  int minFormulasInFinalBlend_ = 0;
  int maxFormulasInFinalBlend_ = 0;
  QVector<library::FormulaType> allowedFormulaTypes_;
  QVector<library::ColoringMethodType> allowedColoringMethodsInBlend_;
  QVector<library::ColoringMethodType> allowedColoringMethodsInFinalBlend_;

  RandomizationSettings() = default;

 public:
  [[nodiscard]] int minBlends() const { return minBlends_; }
  [[nodiscard]] int maxBlends() const { return maxBlends_; }
  [[nodiscard]] int minFormulasInBlend() const { return minFormulasInBlend_; }
  [[nodiscard]] int maxFormulasInBlend() const { return maxFormulasInBlend_; }
  [[nodiscard]] int minFormulasInFinalBlend() const {
    return minFormulasInFinalBlend_;
  }
  [[nodiscard]] int maxFormulasInFinalBlend() const {
    return maxFormulasInFinalBlend_;
  }
  [[nodiscard]] const QVector<library::FormulaType>& allowedFormulaTypes()
      const {
    return allowedFormulaTypes_;
  }
  [[nodiscard]] const QVector<library::ColoringMethodType>&
  allowedColoringMethodsInBlend() const {
    return allowedColoringMethodsInBlend_;
  }
  [[nodiscard]] const QVector<library::ColoringMethodType>&
  allowedColoringMethodsInFinalBlend() const {
    return allowedColoringMethodsInFinalBlend_;
  }

  class Builder {
    std::unique_ptr<RandomizationSettings> settings_;

   public:
    Builder();
    Builder(Builder&) = delete;
    Builder(Builder&&) = delete;

    Builder& setMinBlends(int min) {
      settings_->minBlends_ = min;
      return *this;
    }
    Builder& setMaxBlends(int max) {
      settings_->maxBlends_ = max;
      return *this;
    }
    Builder& setMinFormulasInBlend(int min) {
      settings_->minFormulasInBlend_ = min;
      return *this;
    }
    Builder& setMaxFormulasInBlend(int max) {
      settings_->maxFormulasInBlend_ = max;
      return *this;
    }
    Builder& setMinFormulasInFinalBlend(int min) {
      settings_->minFormulasInFinalBlend_ = min;
      return *this;
    }
    Builder& setMaxFormulasInFinalBlend(int max) {
      settings_->maxFormulasInFinalBlend_ = max;
      return *this;
    }
    Builder& setAllowedFormulaTypes(
        const QVector<library::FormulaType>& types) {
      settings_->allowedFormulaTypes_ = types;
      return *this;
    }
    Builder& setAllowedColoringMethodsInBlend(
        const QVector<library::ColoringMethodType>& methods) {
      settings_->allowedColoringMethodsInBlend_ = methods;
      return *this;
    }
    Builder& setAllowedColoringMethodsInFinalBlend(
        const QVector<library::ColoringMethodType>& methods) {
      settings_->allowedColoringMethodsInFinalBlend_ = methods;
      return *this;
    }

    RandomizationSettings build();
  };
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_RANDOMIZATIONSETTINGS_H
