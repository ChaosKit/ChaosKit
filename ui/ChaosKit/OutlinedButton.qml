import QtQuick 2.12
import QtQuick.Controls 2.12
import ChaosKit 1.0

Button {
  id: outlinedButton
  flat: true

  icon.color: !outlinedButton.enabled
      ? Theme.onSurfaceDisabled
      : Theme.primaryColor

  background: Rectangle {
    implicitWidth: Theme.units(14)
    implicitHeight: Theme.units(6)
    radius: Theme.borderRadius
    color: Theme.buttonColor(outlinedButton)
    border.color: Theme.borderColor
    border.width: 1
  }
}