import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import ChaosKit 1.0

T.Button {
  id: control

  property bool outlined: false
  flat: outlined

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

  function getColor() {
    if (!enabled) return Theme.disabledColor;

    const value =
      control.visualFocus ? 0.12 :
      control.down ? 0.1 :
      control.hovered ? 0.04 :
      0.0;

    if (control.flat) {
      return Qt.rgba(255, 255, 255, value);
    } else {
      return Theme.blend(Theme.primaryColor, Theme.white, value);
    }
  }

  background: Rectangle {
    implicitWidth: Theme.units(14)
    implicitHeight: Theme.units(6)
    visible: !control.flat || control.outlined || control.hovered || control.down
    radius: Theme.borderRadius
    color: getColor()
    border.color: Qt.rgba(255, 255, 255, 0.12)
    border.width: control.outlined ? 1 : 0
  }
}
