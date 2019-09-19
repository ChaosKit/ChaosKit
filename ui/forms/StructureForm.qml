import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.2
import QtQml.Models 2.12
import "../controls"
import "../resources"

ColumnLayout {
  spacing: 0

  DelegateModel {
    id: delegateModel
    model: documentModel
    rootIndex: documentModel.systemIndex
    delegate: BlendListItem {
      Layout.fillWidth: true
      parentModel: delegateModel
    }
  }

  Repeater {
    model: delegateModel
  }

  MouseArea {
    Layout.fillWidth: true
    Layout.fillHeight: true

    onClicked: selectionModel.clear()
  }

  RowLayout {
    Layout.fillWidth: true
    spacing: 0

    SymbolButton {
      symbol: Icons.faPlus
      onClicked: formulaPicker.open()

      Popup {
        id: formulaPicker
        margins: 0
        padding: 4

        FormulaPicker {
          onPicked: {
            documentModel.addBlend(formulaType);
            formulaPicker.close();
          }
        }
      }
    }

    Item {
      Layout.fillWidth: true
    }

    SymbolButton {
      enabled: selectionModel.currentIndex.valid &&
          !documentModel.isFinalBlend(selectionModel.currentIndex)
      symbol: Icons.faTrashAlt
      onClicked: documentModel.removeRowAtIndex(selectionModel.currentIndex)
    }
  }

  ToolOptionsForm {
    id: toolOptions
    Layout.fillWidth: true
    Layout.minimumHeight: 30
    Layout.preferredHeight: 200
  }
}
