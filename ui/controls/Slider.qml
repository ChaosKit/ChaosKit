import QtQuick 2.0
import QtQuick.Controls 2.4
import "../resources"

Slider {
  id: control

  background: Rectangle {
    x: control.leftPadding
    y: control.topPadding
    implicitWidth: 200
    implicitHeight: 20
    width: control.availableWidth
    height: control.availableHeight
    radius: 2
    color: Constants.backgroundColor
    border.color: Constants.mainColor
    border.width: 1

    Rectangle {
      x: 2
      y: 2
      width: control.visualPosition * (parent.width - 4)
      height: parent.height - 4
      color: Constants.mainColor
    }

    Label {
      anchors.centerIn: parent
      text: control.value.toFixed(2)
      font.pointSize: parent.height * 0.5
      font.letterSpacing: 0.5
    }
  }

  handle: Rectangle {
    x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
    y: control.topPadding + (control.availableHeight - height) / 2
    implicitWidth: 3
    implicitHeight: 20
    color: Constants.accentColor
  }
}
