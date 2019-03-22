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

  property bool itemEnabled: true  // TODO: use the model
  property bool open: false
  property var rootModel: null
  property var selectionModel: null
  readonly property var selectionIndex: rootModel.modelIndexForSelection(index)

  MouseArea {
    Layout.fillWidth: true
    Layout.preferredHeight: contents.implicitHeight
    hoverEnabled: true

    onClicked: {
      selectionModel.setCurrentIndex(
          selectionIndex, ItemSelectionModel.ClearAndSelect);
    }

    Rectangle {
      id: background
      anchors.fill: parent
      color: Material.foreground
      opacity: selectionModel.currentIndex == selectionIndex ? 0.25 :
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

        value: weight
        visible: !isFinalBlend
        onMoved: model.weight = value
      }
    }
  }

  // Formulas

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
        id: formulaRepeater
        model: rootModel.childModel(index)
        delegate: FormulaListItem {
          anchors.left: parent.left
          anchors.right: parent.right
          rootModel: itemRoot.rootModel
          blendModel: formulaRepeater.model
          selectionModel: itemRoot.selectionModel
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
