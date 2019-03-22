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

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      TabBar {
        id: editorTabs
        Layout.fillWidth: true

        TabButton {
          text: qsTr("Structure")
        }
        TabButton {
          text: qsTr("Rendering")
        }
      }

      StackLayout {
        currentIndex: editorTabs.currentIndex
        Layout.fillWidth: true
        Layout.margins: 8
        StructureForm {
          id: structureForm
          model: systemModel
        }
        RenderingForm {
          id: renderingForm
        }
      }

      Pane {
        Layout.fillWidth: true
        Layout.minimumHeight: 30
        Layout.preferredHeight: 200

        Label {
          text: "Tool Window"
          anchors.centerIn: parent
        }
      }
    }
  }
}
