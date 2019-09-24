import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.11
import "../controls"

ColumnLayout {
  id: itemRoot
  spacing: 0

  property var parentModel
  readonly property var parentIndex: parentModel.modelIndex(index)

  BlendRow {
    id: row
    modelIndex: parentIndex
    toggleVisible: formulaDelegateModel.count > 1
    Layout.fillWidth: true
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
