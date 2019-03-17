import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import "../controls"
import "../resources"

GridLayout {
  id: itemRoot
  rowSpacing: 0
  columnSpacing: 0
  columns: 4

  property bool itemEnabled: true  // TODO: use the model
  property bool open: false
  property var rootModel: null

  SymbolButton {
    symbol: itemRoot.itemEnabled ? Icons.faEye : Icons.faEyeSlash
    onClicked: itemRoot.itemEnabled = !itemRoot.itemEnabled
  }

  SymbolButton {
    symbol: itemRoot.open ? Icons.faCaretDown : Icons.faCaretRight
    onClicked: itemRoot.open = !itemRoot.open
  }

  Label {
    Layout.fillWidth: true
    Layout.fillHeight: true

    text: display
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
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
        model: rootModel.childModel(index)
        delegate: FormulaListItem {
          anchors.left: parent.left
          anchors.right: parent.right
          rootModel: itemRoot.rootModel
        }
      }

      AddFormulaForm {
        anchors.left: parent.left
        anchors.right: parent.right
        rootModel: itemRoot.rootModel
      }
    }
  }
}
