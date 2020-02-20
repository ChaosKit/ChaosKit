import QtQuick 2.12
import QtQuick.Controls 2.12

IconButton {
  id: button

  background: Rectangle {
    color:
      enabled ?
        Theme.blend(
          Theme.primaryColor, Theme.white,
          button.visualFocus ? 0.12 :
          button.down ? 0.1 :
          button.hovered ? 0.04 :
          0.0
        ) :
      Theme.disabledColor
    radius: button.radius
  }
  flat: false
  icon.color: enabled ? Theme.onPrimaryHigh : Theme.onSurfaceDisabled
  innerOffset: Theme.units(3)
}
