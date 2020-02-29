#ifndef CHAOSKIT_UI_DOCUMENTPROXY_H
#define CHAOSKIT_UI_DOCUMENTPROXY_H

#include <QObject>
#include <QSize>
#include <QString>

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
  Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
  Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)

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

  [[nodiscard]] qreal width() const;
  void setWidth(qreal width);

  [[nodiscard]] qreal height() const;
  void setHeight(qreal height);

 signals:
  void gammaChanged();
  void exposureChanged();
  void vibrancyChanged();
  void colorMapChanged();
  void widthChanged();
  void heightChanged();

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
