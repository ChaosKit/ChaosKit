import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQml.Models 2.11
import "controls"
import "resources"

ListView {  // TODO: make a custom control with selecting etc
  id: rootListView
  delegate: blendDelegate

  Component {
    id: formulaDelegate

    RowLayout {
      id: itemRoot
      spacing: 0

      anchors.left: parent.left
      anchors.right: parent.right

      property bool itemEnabled: true  // TODO: use the model

      SymbolButton {
        symbol: itemRoot.itemEnabled ? Icons.faEye : Icons.faEyeSlash
        onClicked: itemRoot.itemEnabled = !itemRoot.itemEnabled
      }

      Label {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.leftMargin: 20

        text: display
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
      }

      Slider {
        Layout.preferredWidth: 100
        Layout.fillHeight: true

        value: weight
        onMoved: model.weight = value
      }
    }
  }

  Component {
    id: blendDelegate

    GridLayout {
      id: itemRoot
      rowSpacing: 0
      columnSpacing: 0
      columns: 4

      anchors.left: parent.left
      anchors.right: parent.right

      property bool itemEnabled: true  // TODO: use the model
      property bool open: false

      SymbolButton {
        symbol: itemRoot.itemEnabled ? Icons.faEye : Icons.faEyeSlash
        onClicked: itemRoot.itemEnabled = !itemRoot.itemEnabled
      }

      SymbolButton {
        symbol: itemRoot.open ? Icons.faCaretDown : Icons.faCaretRight
        onClicked: itemRoot.open = !itemRoot.open
      }

      Component {
        id: nameLabel

        Label {
          text: display
          verticalAlignment: Text.AlignVCenter
          elide: Text.ElideRight
        }
      }

      Component {
        id: nameTextField

        TextField {
          text: edit
          placeholderText: "(unnamed)"
          onAccepted: model.edit = text
        }
      }

      Loader {
        Layout.fillWidth: true
        Layout.fillHeight: true

        sourceComponent: isFinalBlend ? nameLabel : nameTextField
      }

      Slider {
        Layout.preferredWidth: 100
        Layout.fillHeight: true

        value: weight
        visible: !isFinalBlend
        onMoved: model.weight = value
      }

      // Formulas

      Rectangle {
        Layout.columnSpan: 4
        Layout.fillWidth: true
        Layout.preferredHeight: formulaColumn.implicitHeight

        color: Qt.rgba(0, 0, 0, 0.1)
        visible: itemRoot.open

        Column {
          id: formulaColumn
          spacing: 0
          anchors.fill: parent

          Repeater {
            model: rootListView.model.childModel(index)
            delegate: formulaDelegate
          }

          // Add Formula

          RowLayout {
            spacing: 0

            anchors.left: parent.left
            anchors.right: parent.right

            SymbolButton {
              symbol: Icons.faPlus
            }

            ComboBox {
              Layout.fillWidth: true
              Layout.fillHeight: true
              model: formulaList
            }

            Item {
              Layout.preferredWidth: 80
              Layout.fillHeight: true

              Label {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenterOffset: 10
                font.pixelSize: 9
                font.letterSpacing: 0.45
                text: "1.0"
              }
            }

            SymbolButton {
              symbol: Icons.faLock
            }
          }
        }
      }
    }
  }

}
