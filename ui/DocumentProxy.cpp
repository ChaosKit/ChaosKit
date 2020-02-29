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

QString DocumentProxy::colorMap() {
  return QString::fromStdString(model_->document()->colorMap);
}
void DocumentProxy::setColorMap(const QString& colorMap) {
  model_->setData(model_->documentIndex(), colorMap,
                  DocumentModel::ColorMapRole);
}

qreal DocumentProxy::width() const {
  return static_cast<qreal>(model_->document()->width);
}
void DocumentProxy::setWidth(qreal width) {
  model_->setData(model_->documentIndex(), static_cast<uint>(width),
                  DocumentModel::WidthRole);
}

qreal DocumentProxy::height() const {
  return static_cast<qreal>(model_->document()->height);
}
void DocumentProxy::setHeight(qreal height) {
  model_->setData(model_->documentIndex(), static_cast<uint>(height),
                  DocumentModel::HeightRole);
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
      case DocumentModel::ColorMapRole:
        emit colorMapChanged();
        break;
      case DocumentModel::HeightRole:
        emit heightChanged();
        break;
      case DocumentModel::WidthRole:
        emit widthChanged();
        break;
      default:;
    }
  }
}
void DocumentProxy::onDocumentReset() {
  emit gammaChanged();
  emit exposureChanged();
  emit vibrancyChanged();
  emit colorMapChanged();
  emit widthChanged();
  emit heightChanged();
}

}  // namespace chaoskit::ui
