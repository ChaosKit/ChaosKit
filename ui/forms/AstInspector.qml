import QtQuick
import QtQuick.Controls
import QtQuick.Window
import ChaosKit

Popup {
  id: popup
  width: 400
  height: 400
  modal: true
  visible: true

  property string modelSource
  property string astSource

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

  TabBar {
    id: tabs
    anchors.top: title.bottom
    anchors.topMargin: Theme.units(3)
    width: parent.width

    TabButton {
      text: "Model"
    }
    TabButton {
      text: "AST"
    }
  }

  ScrollView {
    anchors.top: tabs.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    TextArea {
      id: sourceArea
      font.family: monospaceFont.family
      font.pointSize: 11
      readOnly: true
      text: tabs.currentIndex === 0 ? modelSource : astSource
    }
  }
}
