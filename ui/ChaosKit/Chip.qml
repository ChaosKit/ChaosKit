import QtQuick

Rectangle {
  id: chip

  property alias text: label.text

  color: Theme.alpha(Theme.onSurface, Drag.active ? 0.2 : 0.12)
  implicitHeight: label.height
  implicitWidth: label.implicitWidth
  radius: Theme.borderRadius

  TextLabel {
    id: label
    fontSize: Theme.captionFontSize
    leftPadding: Theme.smallPadding
    height: Theme.chipHeight
    rightPadding: Theme.smallPadding
    verticalAlignment: Text.AlignVCenter
  }
}
