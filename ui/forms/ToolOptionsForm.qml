import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Pane {
  property var rootModel: null
  property var selectionModel: null

  Label {
    id: label
    text: "Nothing selected"
    anchors.centerIn: parent
  }

  Connections {
    target: selectionModel
    onCurrentChanged: {
      if (!current.valid) {
        label.text = "Nothing selected";
      } else if (current.parent.valid) {
        label.text = `Formula number ${current.row}`;
      } else {
        label.text = `Blend number ${current.row}`;
      }
    }
  }
}
