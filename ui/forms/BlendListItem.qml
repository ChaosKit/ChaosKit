import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.11
import app.chaoskit 1.0
import "../controls"
import "../resources"

ColumnLayout {
  id: itemRoot
  spacing: 0

  property var parentModel
  readonly property var parentIndex: parentModel.modelIndex(index)

  states: [
    State {
      name: "open"

      PropertyChanges {
        target: formulaList
        visible: true
      }
      PropertyChanges {
        target: openToggle
        symbol: Icons.faCaretDown
      }
    }
  ]

  // Close formulas if they should not be visible.
  Connections {
    target: documentModel
    onRowsRemoved: {
      if (parent === parentIndex && documentModel.rowCount(parent) <= 1) {
        itemRoot.state = "";
      }
    }
  }

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
        symbol: model.enabled ? Icons.faEye : Icons.faEyeSlash
        onClicked: model.enabled = !model.enabled
      }

      SymbolButton {
        id: openToggle
        symbol: Icons.faCaretRight
        visible: formulaDelegateModel.count > 1
        onClicked: {
          itemRoot.state = (itemRoot.state === "open") ? "" : "open";
        }
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
        visible: type !== DocumentEntryType.FinalBlend
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
    id: formulaList

    color: Qt.rgba(0, 0, 0, 0.1)
    implicitHeight: formulaColumn.implicitHeight
    visible: false
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
