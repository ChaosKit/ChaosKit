import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import ChaosKit

T.RoundButton {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding)

  property bool outlined: false
  flat: outlined

  padding: 0
  spacing: 0

  icon.width: Theme.iconSize
  icon.height: Theme.iconSize
  icon.color: !control.enabled
      ? Theme.onSurfaceDisabled
      : control.flat
          ? Theme.primaryColor
          : Theme.onPrimaryHigh

  contentItem: IconLabel {
    spacing: control.spacing
    mirrored: control.mirrored
    display: control.display

    icon: control.icon
    text: control.text
    font: control.font
    color: !control.enabled
        ? Theme.onSurfaceDisabled
        : control.flat
            ? Theme.primaryColor
            : Theme.onPrimaryHigh
  }

  background: Rectangle {
    implicitWidth: Theme.units(10)
    implicitHeight: Theme.units(10)
    radius: control.radius
    visible: !control.flat || control.outlined || control.hovered || control.down
    color: Theme.buttonColor(control)
    border.color: Qt.rgba(255, 255, 255, 0.12)
    border.width: control.outlined ? 1 : 0
  }
}
