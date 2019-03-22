import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import app.chaoskit 1.0

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

  Drawer {
    id: drawer
    edge: Qt.RightEdge
    width: 300
    height: root.height
    modal: false
    visible: true

    TabBar {
      id: editorTabs
      anchors.top: parent.top
      width: parent.width

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
