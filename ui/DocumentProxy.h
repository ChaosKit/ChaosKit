#ifndef CHAOSKIT_UI_DOCUMENTPROXY_H
#define CHAOSKIT_UI_DOCUMENTPROXY_H

#include <QObject>

namespace chaoskit::ui {

class DocumentModel;

class DocumentProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY gammaChanged)
  Q_PROPERTY(
      float exposure READ exposure WRITE setExposure NOTIFY exposureChanged)
  Q_PROPERTY(
      float vibrancy READ vibrancy WRITE setVibrancy NOTIFY vibrancyChanged)
  Q_PROPERTY(
      QString colorMap READ colorMap WRITE setColorMap NOTIFY colorMapChanged)
 public:
  explicit DocumentProxy(DocumentModel* model);

  float gamma();
  void setGamma(float gamma);

  float exposure();
  void setExposure(float exposure);

  float vibrancy();
  void setVibrancy(float vibrancy);

  QString colorMap();
  void setColorMap(const QString& colorMap);

 signals:
  void gammaChanged();
  void exposureChanged();
  void vibrancyChanged();
  void colorMapChanged();

 private:
  DocumentModel* model_;

 private slots:
  void onModelDataChanged(const QModelIndex& topLeft,
                          const QModelIndex& bottomRight,
                          const QVector<int>& roles);
  void onDocumentReset();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_DOCUMENTPROXY_H
