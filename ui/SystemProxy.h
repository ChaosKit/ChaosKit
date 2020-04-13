#ifndef CHAOSKIT_UI_SYSTEMPROXY_H
#define CHAOSKIT_UI_SYSTEMPROXY_H

#include <QObject>

namespace chaoskit::ui {

class DocumentModel;

class SystemProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(int ttl READ ttl WRITE setTtl NOTIFY ttlChanged)
 public:
  explicit SystemProxy(DocumentModel* model);

  [[nodiscard]] int ttl() const;
  void setTtl(int ttl);

 signals:
  void ttlChanged();

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
