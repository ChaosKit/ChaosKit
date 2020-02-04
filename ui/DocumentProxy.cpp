#include "DocumentProxy.h"
#include <QDebug>
#include "DocumentModel.h"

namespace chaoskit::ui {

DocumentProxy::DocumentProxy(DocumentModel* model)
    : QObject(model), model_(model) {
  connect(model, &QAbstractItemModel::dataChanged, this,
          &DocumentProxy::onModelDataChanged);
  connect(model, &DocumentModel::documentReset, this,
          &DocumentProxy::onDocumentReset);
}

float DocumentProxy::gamma() { return model_->document()->gamma; }
void DocumentProxy::setGamma(float gamma) {
  model_->setData(model_->documentIndex(), gamma, DocumentModel::GammaRole);
}

float DocumentProxy::exposure() { return model_->document()->exposure; }
void DocumentProxy::setExposure(float exposure) {
  model_->setData(model_->documentIndex(), exposure,
                  DocumentModel::ExposureRole);
}

float DocumentProxy::vibrancy() { return model_->document()->vibrancy; }
void DocumentProxy::setVibrancy(float vibrancy) {
  model_->setData(model_->documentIndex(), vibrancy,
                  DocumentModel::VibrancyRole);
}

void DocumentProxy::onModelDataChanged(const QModelIndex& topLeft,
                                       const QModelIndex& bottomRight,
                                       const QVector<int>& roles) {
  auto documentIndex = model_->documentIndex();
  if (topLeft != documentIndex && bottomRight != documentIndex) {
    return;
  }

  for (int role : roles) {
    switch (role) {
      case DocumentModel::GammaRole:
        emit gammaChanged();
        break;
      case DocumentModel::ExposureRole:
        emit exposureChanged();
        break;
      case DocumentModel::VibrancyRole:
        emit vibrancyChanged();
        break;
      default:;
    }
  }
}

void DocumentProxy::onDocumentReset() {
  emit gammaChanged();
  emit exposureChanged();
  emit vibrancyChanged();
}

}  // namespace chaoskit::ui
