import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import ChaosKit 1.0

T.TabButton {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding)

  horizontalPadding: Theme.units(3)
  verticalPadding: 0
  spacing: 0

  font.pointSize: Theme.buttonFontSize
  font.letterSpacing: Theme.letterSpacing(Theme.buttonFontSize)
  font.weight: Font.DemiBold

  icon.width: Theme.iconSize
  icon.height: Theme.iconSize
  icon.color: checked ? Theme.primaryColor : Theme.onSurfaceHigh

  contentItem: IconLabel {
      spacing: control.spacing
      mirrored: control.mirrored
      display: control.display

      icon: control.icon
      text: control.text
      font: control.font
      color: checked ? Theme.primaryColor : Theme.onSurfaceHigh
  }

  function getColor() {
    const value =
      control.visualFocus ? 0.12 :
      control.down ? 0.1 :
      control.hovered ? 0.04 :
      0.0;

    return Qt.rgba(255, 255, 255, value);
  }

  background: Rectangle {
    implicitHeight: Theme.units(14)
    visible: control.hovered || control.down
    color: getColor()
  }
}
