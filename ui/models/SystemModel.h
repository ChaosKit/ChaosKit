#ifndef CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
#define CHAOSKIT_UI_MODELS_SYSTEMMODEL_H

#include <chaoskit.pb.h>
#include <QObject>
#include "BaseModel.h"

namespace chaoskit::ui {

class SystemModel : public QObject, public BaseModel<System> {
  Q_OBJECT
  Q_PROPERTY(const QString& astSource READ astSource NOTIFY somethingChanged);
  Q_PROPERTY(
      const QString& modelSource READ modelSource NOTIFY somethingChanged);

 public:
  SystemModel(QObject* parent = nullptr)
      : QObject(parent), BaseModel<System>() {}

  [[nodiscard]] QString astSource() const;
  [[nodiscard]] QString modelSource() const;

 signals:
  void somethingChanged();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
