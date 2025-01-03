import QtQuick
import QtQuick.Controls

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
  size: Theme.iconSize
}
