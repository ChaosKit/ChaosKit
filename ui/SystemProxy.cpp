#include "SystemProxy.h"
#include "DocumentModel.h"

namespace chaoskit::ui {

SystemProxy::SystemProxy(DocumentModel* model) : QObject(model), model_(model) {
  connect(model, &QAbstractItemModel::dataChanged, this,
          &SystemProxy::onModelDataChanged);
  connect(model, &DocumentModel::systemReset, this,
          &SystemProxy::onSystemReset);
}

int SystemProxy::ttl() const { return model_->system()->ttl; }
void SystemProxy::setTtl(int ttl) {
  model_->setData(model_->systemIndex(), ttl, DocumentModel::TtlRole);
}

QModelIndex SystemProxy::isolatedBlendIndex() const {
  return model_
      ->data(model_->systemIndex(), DocumentModel::IsolatedBlendIndexRole)
      .toModelIndex();
}
void SystemProxy::setIsolatedBlendIndex(const QModelIndex& blendIndex) {
  model_->setData(model_->systemIndex(), blendIndex,
                  DocumentModel::IsolatedBlendIndexRole);
}

void SystemProxy::onModelDataChanged(const QModelIndex& topLeft,
                                     const QModelIndex& bottomRight,
                                     const QVector<int>& roles) {
  auto systemIndex = model_->systemIndex();
  if (topLeft != systemIndex && bottomRight != systemIndex) {
    return;
  }

  for (int role : roles) {
    switch (role) {
      case DocumentModel::TtlRole:
        emit ttlChanged();
        break;
      case DocumentModel::IsolatedBlendIndexRole:
        emit isolatedBlendIndexChanged();
        break;
      default:;
    }
  }
}
void SystemProxy::onSystemReset() {
  emit ttlChanged();
  emit isolatedBlendIndexChanged();
}

}  // namespace chaoskit::ui
