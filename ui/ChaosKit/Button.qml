import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import ChaosKit

T.Button {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding)

  font.capitalization: Font.AllUppercase
  font.pointSize: Theme.buttonFontSize
  font.letterSpacing: Theme.letterSpacing(Theme.buttonFontSize)
  font.weight: Font.DemiBold

  padding: 0
  leftPadding: (icon.name || icon.source.toString()) ? Theme.units(2) : Theme.units(3)
  rightPadding: Theme.units(3)
  spacing: Theme.units(1)

  icon.width: Theme.smallIconSize
  icon.height: Theme.smallIconSize
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
    implicitWidth: Theme.units(14)
    implicitHeight: Theme.units(6)
    radius: Theme.borderRadius
    color: Theme.buttonColor(control)
  }
}
