#ifndef CHAOSKIT_UI_MODELS_PROJECTMODEL_H
#define CHAOSKIT_UI_MODELS_PROJECTMODEL_H

#include <QObject>
#include <QUrl>
#include "BaseModel.h"
#include "ColorMapModel.h"
#include "ModelFactory.h"
#include "SystemModel.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class ProjectModel : public BaseModel<Project> {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::ColorMapModel* colorMap READ colorMap NOTIFY
                 colorMapChanged);
  Q_PROPERTY(
      chaoskit::ui::SystemModel* system READ system NOTIFY systemChanged);
  Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY gammaChanged);
  Q_PROPERTY(
      float exposure READ exposure WRITE setExposure NOTIFY exposureChanged);
  Q_PROPERTY(
      float vibrancy READ vibrancy WRITE setVibrancy NOTIFY vibrancyChanged);
  Q_PROPERTY(uint width READ width WRITE setWidth NOTIFY widthChanged);
  Q_PROPERTY(uint height READ height WRITE setHeight NOTIFY heightChanged);
  Q_PROPERTY(bool modified READ modified NOTIFY modifiedChanged);
  Q_PROPERTY(const QString& name READ name NOTIFY nameChanged);
  Q_PROPERTY(const QUrl& fileUrl READ fileUrl NOTIFY fileUrlChanged);

 public:
  explicit ProjectModel(ModelFactory* modelFactory, QObject* parent = nullptr);

  void setProto(Project* proto) override;

  [[nodiscard]] ColorMapModel* colorMap() const { return colorMapModel_; }
  [[nodiscard]] SystemModel* system() const { return systemModel_; }

  [[nodiscard]] float gamma() const { return proto_->gamma(); }
  void setGamma(float gamma);

  [[nodiscard]] float exposure() const { return proto_->exposure(); }
  void setExposure(float exposure);

  [[nodiscard]] float vibrancy() const { return proto_->vibrancy(); }
  void setVibrancy(float vibrancy);

  [[nodiscard]] uint width() const { return proto_->width(); }
  void setWidth(uint width);

  [[nodiscard]] uint height() const { return proto_->height(); }
  void setHeight(uint height);

  // File-related methods

  [[nodiscard]] const QString& name() const { return name_; }
  [[nodiscard]] const QUrl& fileUrl() const { return fileUrl_; }
  [[nodiscard]] bool modified() const { return modified_; }

  Q_INVOKABLE bool loadFromUrl(const QUrl& url);
  Q_INVOKABLE bool saveToUrl(const QUrl& url);

 signals:
  void projectChanged();
  void gammaChanged();
  void exposureChanged();
  void vibrancyChanged();
  void widthChanged();
  void heightChanged();
  void colorMapChanged();
  void systemChanged();
  void modifiedChanged();
  void nameChanged();
  void fileUrlChanged();
  void fileIoFailed(const QString& error);

 private:
  ColorMapModel* colorMapModel_;
  SystemModel* systemModel_;
  bool modified_ = true;
  QString name_ = "Untitled";
  QUrl fileUrl_;

 private slots:
  void setFileUrl(const QUrl& url);
  void markAsModified();
  void markAsUnmodified();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_PROJECTMODEL_H
