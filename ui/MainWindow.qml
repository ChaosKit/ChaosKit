import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import app.chaoskit 1.0

ApplicationWindow {
  id: root
  title: "ChaosKit"
  width: 800
  height: 600
  visible: true

  Material.theme: Material.Dark
  Material.primary: Material.BlueGrey
  Material.accent: Material.LightGreen
  Material.background: Material.color(Material.BlueGrey, Material.Shade800)
  Material.foreground: Material.White

  SystemModel {
    id: systemModel
  }

  RowLayout {
    spacing: 0
    anchors.fill: parent

    Editor {
      id: editor
      refreshInterval: 100
      gamma: renderingForm.gamma
      exposure: renderingForm.exposure
      vibrancy: renderingForm.vibrancy

      Layout.fillWidth: true
      Layout.fillHeight: true
    }

    Item {
      Layout.fillHeight: true
      Layout.minimumWidth: 240
      Layout.preferredWidth: 240

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

        Rectangle {
          color: Material.color(Material.BlueGrey, Material.Shade900)
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
