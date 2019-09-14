import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.11
import "../controls"
import "../resources"

ColumnLayout {
  id: itemRoot
  spacing: 0

  property var parentModel
  property bool itemEnabled: true  // TODO: use the model
  property bool open: false
  readonly property var parentIndex: parentModel.modelIndex(index)

  MouseArea {
    Layout.fillWidth: true
    Layout.preferredHeight: contents.implicitHeight
    hoverEnabled: true

    onClicked: {
      selectionModel.setCurrentIndex(
          parentIndex, ItemSelectionModel.ClearAndSelect);
    }

    Rectangle {
      id: background
      anchors.fill: parent
      color: Material.foreground
      opacity: selectionModel.currentIndex == parentIndex ? 0.25 :
        (parent.containsMouse ? 0.1 : 0.0)
    }

    RowLayout {
      id: contents
      anchors.left: parent.left
      anchors.right: parent.right

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

        value: weight || 0
        visible: !documentModel.isFinalBlend(parentIndex)
        onMoved: model.weight = value
      }
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
    Layout.fillWidth: true
    Layout.preferredHeight: formulaColumn.implicitHeight

    color: Qt.rgba(0, 0, 0, 0.1)
    visible: itemRoot.open

    Column {
      id: formulaColumn
      spacing: 0
      anchors.fill: parent

      Repeater {
        model: formulaDelegateModel
      }

      AddFormulaForm {
        anchors.left: parent.left
        anchors.right: parent.right
        blendIndex: parentIndex
      }
    }
  }
}
