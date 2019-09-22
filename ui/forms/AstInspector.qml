import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import "../controls"

Popup {
  id: popup
  width: 400
  height: 400
  modal: true
  visible: true

  property alias source: sourceArea.text

  Label {
    id: title
    x: 4
    y: 4
    text: "AST Inspector"
    font.bold: true
  }

  IconButton {
    iconName: "times"
    y: -8
    anchors.right: parent.right
    anchors.rightMargin: -8
    onClicked: popup.close()
  }

  ScrollView {
    anchors.top: title.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: 4
    contentWidth: sourceArea.contentWidth
    contentHeight: sourceArea.contentHeight

    TextArea {
      id: sourceArea
      font.family: monospaceFont.family
      font.pointSize: 12
      readOnly: true
    }
  }
}
