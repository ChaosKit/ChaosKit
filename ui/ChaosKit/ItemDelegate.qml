import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T

T.ItemDelegate {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding,
                           implicitIndicatorHeight + topPadding + bottomPadding)

  padding: Theme.units(2)
  spacing: Theme.units(1)

  icon.width: Theme.smallIconSize
  icon.height: Theme.smallIconSize
  icon.color: Theme.onSurfaceHigh

  contentItem: IconLabel {
    spacing: control.spacing
    mirrored: control.mirrored
    display: control.display
    alignment: control.display === IconLabel.IconOnly || control.display === IconLabel.TextUnderIcon ? Qt.AlignCenter : Qt.AlignLeft

    icon: control.icon
    text: control.text
    font: control.font
    color: Theme.onSurfaceHigh
  }

  function getColor() {
    if (!enabled) return Theme.disabledColor;
    const value =
      (control.visualFocus || control.highlighted) ? 0.12 :
      control.down ? 0.1 :
      control.hovered ? 0.04 :
      0.0;

    return Qt.rgba(255, 255, 255, value);
  }

  background: Rectangle {
    implicitWidth: 100
    implicitHeight: 28
    visible: control.down || control.highlighted || control.visualFocus
    color: getColor()
  }
}
