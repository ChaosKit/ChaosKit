import QtQuick 2.0
import QtQuick.Controls 2.4 as C
import "../resources"

C.Button {
  id: button
  font.pixelSize: Constants.fontSize

  padding: 4
  leftPadding: padding
  rightPadding: padding

  background: Rectangle {
    implicitWidth: Constants.fontSize + (button.padding * 2)
    implicitHeight: Constants.fontSize + (button.padding * 2)

    visible: !button.flat || button.down || button.checked || button.highlighted
    color: Constants.mainColor  // TODO: handle all the states

    border.color: Constants.accentColor
    border.width: button.visualFocus ? 2 : 0
  }
}
