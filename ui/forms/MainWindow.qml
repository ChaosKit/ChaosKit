import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
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

  RowLayout {
    spacing: 0
    anchors.fill: parent

    Editor {
      id: editor
      system: systemModel.system
      refreshInterval: 100
      gamma: renderingForm.gamma
      exposure: renderingForm.exposure
      vibrancy: renderingForm.vibrancy

      Layout.fillWidth: true
      Layout.fillHeight: true

      Component.onCompleted: {
        running = true;
      }
    }

    Item {
      Layout.fillHeight: true
      Layout.minimumWidth: 240
      Layout.preferredWidth: 300

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
}
