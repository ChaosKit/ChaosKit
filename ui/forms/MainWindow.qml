import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import app.chaoskit 1.0
import "../controls"

ApplicationWindow {
  id: root
  title: "ChaosKit"
  font.family: "Inter"
  width: 1024
  height: 768
  visible: true

  Connections {
    target: selectionModel
    onCurrentChanged: {
      editor.selectedElement = documentModel.entryAtIndex(current);
    }
  }

  Editor {
    id: editor
    refreshInterval: 100
    gamma: 2.2 //renderingForm.gamma
    exposure: 0.0 //renderingForm.exposure
    vibrancy: 0.0 //renderingForm.vibrancy

    anchors.fill: parent
    transform: Translate {
      x: drawer.position * drawer.width * -0.33
    }

    Component.onCompleted: {
      running = true;
    }
  }

  IconButton {
    id: showButton
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.topMargin: 5
    anchors.rightMargin: 4
    iconName: "chevron-left"
    onClicked: drawer.open()
  }

  Button {
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.leftMargin: 8
    anchors.bottomMargin: 4
    text: "AST Inspector"

    onClicked: astInspectorLoader.active = true

    Loader {
      id: astInspectorLoader
      active: false
      sourceComponent: AstInspector {
        anchors.centerIn: Overlay.overlay
        source: documentModel.debugSource

        onClosed: astInspectorLoader.active = false
      }
    }
  }

  Drawer {
    id: drawer
    closePolicy: Popup.NoAutoClose
    edge: Qt.RightEdge
    width: 300
    height: root.height
    modal: false
    visible: true

    IconButton {
      id: hideButton
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.topMargin: 5
      anchors.leftMargin: 4
      iconName: "chevron-right"
      onClicked: drawer.close()
    }

    ColumnLayout {
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.topMargin: 40

      Label {
        text: 'TBD'
      }
    }
  }
}
