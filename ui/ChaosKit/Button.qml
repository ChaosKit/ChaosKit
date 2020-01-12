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
  leftPadding: (icon.name || icon.source.toString()) ? Theme.units(3) : Theme.units(4)
  rightPadding: Theme.units(4)
  spacing: Theme.units(2)

  icon.width: Theme.units(4)
  icon.height: Theme.units(4)
  icon.color: control.checked || control.highlighted ? control.palette.brightText :
              control.flat && !control.down ? (control.visualFocus ? control.palette.highlight : control.palette.windowText) : control.palette.buttonText

  contentItem: IconLabel {
      spacing: control.spacing
      mirrored: control.mirrored
      display: control.display

      icon: control.icon
      text: control.text
      font: control.font
      color: control.flat ? Theme.primaryColor : Theme.onPrimaryHigh
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
      implicitWidth: Theme.units(16)
      implicitHeight: Theme.units(8)
      visible: !control.flat || control.outlined || control.hovered || control.down
      radius: Theme.borderRadius
      color: getColor()
      border.color: Qt.rgba(255, 255, 255, 0.12)
      border.width: control.outlined ? 1 : 0
  }
}
