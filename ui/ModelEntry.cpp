#include "ModelEntry.h"
#include <QMetaProperty>

namespace chaoskit::ui {

ModelEntry::ModelEntry(QAbstractItemModel *model, const QModelIndex &index)
    : QQmlPropertyMap(this, model),
      model_(model),
      index_(index),
      element_(nullptr) {
  QHash<int, QByteArray> roleNames = model->roleNames();
  for (auto it = roleNames.cbegin(); it != roleNames.cend(); ++it) {
    QString name = QString::fromUtf8(*it);
    insert(name, model->data(index, it.key()));
    reverseRoleNames_.insert(name, it.key());
  }

  connect(model, &QAbstractItemModel::dataChanged, this,
          &ModelEntry::handleDataChanged);
  connect(this, &QQmlPropertyMap::valueChanged, this,
          &ModelEntry::handleValueChanged);
}

void ModelEntry::proxyElement(QObject *element) {
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

void ModelEntry::handleValueChanged(const QString &key, const QVariant &value) {
  if (reverseRoleNames_.contains(key)) {
    model_->setData(index_, value, reverseRoleNames_.value(key));
  } else {
    element_->setProperty(key.toUtf8(), value);
  }
}

void ModelEntry::handleDataChanged(const QModelIndex &topLeft,
                                   const QModelIndex &bottomRight,
                                   const QVector<int> &roles) {
  if (topLeft != index_ && bottomRight != index_) return;
  auto roleNames = model_->roleNames();

  for (int role : roles) {
    auto roleName = QString::fromUtf8(roleNames.value(role));
    insert(roleName, model_->data(index_, role));
  }
}

}  // namespace chaoskit::ui
