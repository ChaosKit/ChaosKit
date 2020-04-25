import QtQuick 2.12

Rectangle {
  id: chip

  property alias text: label.text

  color: Theme.alpha(Theme.onSurface, Drag.active ? 0.2 : 0.12)
  implicitHeight: label.height
  implicitWidth: label.implicitWidth
  radius: Theme.borderRadius

  Label {
    id: label
    font.pointSize: Theme.captionFontSize
    font.letterSpacing: Theme.letterSpacing(Theme.captionFontSize)
    leftPadding: Theme.smallPadding
    height: Theme.chipHeight
    rightPadding: Theme.smallPadding
    verticalAlignment: Text.AlignVCenter
  }
}
