import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.12

GridLayout {
  id: root
  columns: 2
  rowSpacing: 4
  columnSpacing: 4

  signal picked(string formulaType)

  Repeater {
    model: formulaList
    delegate: FormulaCard {
      type: modelData
      onClicked: root.picked(modelData)
    }
  }
}
