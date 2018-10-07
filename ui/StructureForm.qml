import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQml.Models 2.11
import "Theme.js" as Theme

ListView {
  id: rootListView

  Component {
    id: blendDelegate

    GridLayout {
      columns: 2
      rowSpacing: 0
      anchors.left: parent.left
      anchors.right: parent.right

      property bool open: false

      Button {
        text: open ? "Close" : "Open"
        flat: true
        onClicked: open = !open
      }
      Label {
        text: model.display
        Layout.fillWidth: true
      }

      ColumnLayout {
        Layout.fillWidth: true
        Layout.columnSpan: 2
        visible: open
        spacing: 0

        Repeater {
          model: rootListView.model.childModel(index)
          delegate: Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32

            color: Theme.backgroundColor

            Label {
              text: display
            }
          }
        }
      }
    }
  }

  delegate: blendDelegate
}
