#ifndef CHAOSKIT_UI_SYSTEM_H
#define CHAOSKIT_UI_SYSTEM_H

#include <core/System.h>
#include <QMetaType>
#include <QObject>

Q_DECLARE_METATYPE(chaoskit::core::System);

namespace chaoskit {
namespace ui {

class System : public QObject {
  Q_OBJECT
 public:
  System();

  const core::System &system() const { return internal_system_; }

 private:
  core::System internal_system_;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_SYSTEM_H
