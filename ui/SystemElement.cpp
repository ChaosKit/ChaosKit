#include "SystemElement.h"
#include <QMetaProperty>
#include "SystemModel.h"

namespace chaoskit::ui {

SystemElement::SystemElement(SystemModel *model, const QModelIndex &index)
    : QQmlPropertyMap(this, model), model_(model), index_(index) {
  auto roleNames = model->roleNames();
  for (auto it = roleNames.cbegin(); it != roleNames.cend(); ++it) {
    QString name = QString::fromUtf8(*it);
    insert(name, model->data(index, it.key()));
    reverseRoleNames_.insert(name, it.key());
  }

  connect(this, &QQmlPropertyMap::valueChanged, this,
          &SystemElement::handleValueChanged);
}

void SystemElement::proxyElement(QObject *element) {
  element_ = element;
  auto *metaObject = element->metaObject();
  for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount();
       i++) {
    const auto &metaProperty = metaObject->property(i);
    QString key = QString::fromUtf8(metaProperty.name());
    if (!reverseRoleNames_.contains(key)) {
      insert(key, metaProperty.read(element));
    }
  }
}

void SystemElement::handleValueChanged(const QString &key,
                                       const QVariant &value) {
  if (reverseRoleNames_.contains(key)) {
    model_->setData(index_, value, reverseRoleNames_.value(key));
  } else {
    element_->setProperty(key.toUtf8(), value);
  }
}

}  // namespace chaoskit::ui
