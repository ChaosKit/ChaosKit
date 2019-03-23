import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import app.chaoskit 1.0
import "../controls"
import "../resources"

ApplicationWindow {
  id: root
  title: "ChaosKit"
  width: 1024
  height: 768
  visible: true

  SystemModel {
    id: systemModel
  }

  Editor {
    id: editor
    system: systemModel.system
    refreshInterval: 100
    gamma: renderingForm.gamma
    exposure: renderingForm.exposure
    vibrancy: renderingForm.vibrancy

    anchors.fill: parent
    transform: Translate {
      x: drawer.position * drawer.width * -0.33
    }

    Component.onCompleted: {
      running = true;
    }
  }

  SymbolButton {
    id: showButton
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.topMargin: 5
    anchors.rightMargin: 4
    symbol: Icons.faChevronLeft
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
        source: systemModel.source

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

    SymbolButton {
      id: hideButton
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.topMargin: 5
      anchors.leftMargin: 4
      symbol: Icons.faChevronRight
      onClicked: drawer.close()
    }

    TabBar {
      id: editorTabs
      anchors.top: parent.top
      anchors.left: hideButton.right
      anchors.right: parent.right
      anchors.leftMargin: 4

      TabButton {
        text: qsTr("Structure")
      }
      TabButton {
        text: qsTr("Rendering")
      }
    }

    StackLayout {
      currentIndex: editorTabs.currentIndex
      anchors.top: editorTabs.bottom
      anchors.bottom: parent.bottom
      width: parent.width

      StructureForm {
        id: structureForm
        model: systemModel
      }
      RenderingForm {
        id: renderingForm
      }
    }
  }
}
