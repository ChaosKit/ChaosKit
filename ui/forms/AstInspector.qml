import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import ChaosKit 1.0

Popup {
  id: popup
  width: 400
  height: 400
  modal: true
  visible: true

  property alias source: sourceArea.text

  Label {
    id: title
    text: "Debug Source"
    font.letterSpacing: Theme.letterSpacing(Theme.headline6FontSize)
    font.pointSize: Theme.headline6FontSize
    font.weight: Font.Medium
  }

  IconButton {
    iconName: "close"
    anchors.top: parent.top
    anchors.topMargin: -6
    anchors.right: parent.right
    anchors.rightMargin: Theme.units(-2)
    onClicked: popup.close()
  }

  ScrollView {
    anchors.top: title.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.topMargin: Theme.units(3)

    TextArea {
      id: sourceArea
      font.family: monospaceFont.family
      font.pointSize: 11
      readOnly: true
    }
  }
}
