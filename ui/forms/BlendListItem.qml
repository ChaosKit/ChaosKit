import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.11
import app.chaoskit 1.0
import "../controls"

DropArea {
  id: root

  property var parentModel
  readonly property var parentIndex: parentModel.modelIndex(index)
  property bool acceptsDrag: containsDrag && drag.source !== root

  implicitHeight: contents.implicitHeight
  Drag.active: row.drag.active

  onDropped: {
    if (root.acceptsDrag) {
      documentModel.absorbBlend(drop.source.parentIndex, root.parentIndex);
    }
  }

  ColumnLayout {
    anchors.left: parent.left
    anchors.right: parent.right
    id: contents
    spacing: 0

    BlendRow {
      id: row
      drag.target: root
      drag.axis: Drag.YAxis
      modelIndex: parentIndex
      toggleVisible:
        formulaDelegateModel.count > (type === DocumentEntryType.Blend ? 1 : 0)
      Layout.fillWidth: true

      onDragEnded: {
        root.Drag.drop();
      }
    }

    // Formulas

    DelegateModel {
      id: formulaDelegateModel
      model: documentModel
      rootIndex: parentIndex
      delegate: FormulaListItem {
        anchors.left: parent.left
        anchors.right: parent.right
        parentModel: formulaDelegateModel
      }
    }

    Rectangle {
      id: formulaList

      color: Qt.rgba(0, 0, 0, 0.1)
      implicitHeight: formulaColumn.implicitHeight
      visible: row.isOpen
      Layout.fillWidth: true

      Column {
        id: formulaColumn
        spacing: 0
        anchors.fill: parent

        Repeater {
          model: formulaDelegateModel
        }
      }
    }
  }

  Rectangle {
    anchors.fill: parent
    border.width: 1
    border.color: Material.foreground
    color: "transparent"
    visible: acceptsDrag
  }
}
