#ifndef CHAOSKIT_UI_SYSTEMPROXY_H
#define CHAOSKIT_UI_SYSTEMPROXY_H

#include <QObject>

namespace chaoskit::ui {

class DocumentModel;

class SystemProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(int ttl READ ttl WRITE setTtl NOTIFY ttlChanged)
  Q_PROPERTY(int skip READ skip WRITE setSkip NOTIFY skipChanged)
  Q_PROPERTY(QModelIndex isolatedBlendIndex READ isolatedBlendIndex WRITE
                 setIsolatedBlendIndex NOTIFY isolatedBlendIndexChanged)
 public:
  explicit SystemProxy(DocumentModel* model);

  [[nodiscard]] int ttl() const;
  void setTtl(int ttl);

  [[nodiscard]] int skip() const;
  void setSkip(int skip);

  [[nodiscard]] QModelIndex isolatedBlendIndex() const;
  void setIsolatedBlendIndex(const QModelIndex& blendIndex);

 signals:
  void ttlChanged();
  void skipChanged();
  void isolatedBlendIndexChanged();

 private:
  DocumentModel* model_;

 private slots:
  void onModelDataChanged(const QModelIndex& topLeft,
                          const QModelIndex& bottomRight,
                          const QVector<int>& roles);
  void onSystemReset();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_SYSTEMPROXY_H
