import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.12
import "../controls"
import "../resources"

ColumnLayout {
  id: root
  spacing: 0

  property alias model: repeater.model

  ItemSelectionModel {
    id: rootSelectionModel
    model: repeater.model
  }

  Repeater {
    id: repeater
    delegate: BlendListItem {
      Layout.fillWidth: true
      rootModel: repeater.model
      selectionModel: rootSelectionModel
    }
  }

  MouseArea {
    Layout.fillWidth: true
    Layout.fillHeight: true

    onClicked: rootSelectionModel.clear()
  }

  RowLayout {
    Layout.fillWidth: true
    spacing: 0

    SymbolButton {
      symbol: Icons.faPlus
      onClicked: model.addBlend()
    }

    Item {
      Layout.fillWidth: true
    }

    SymbolButton {
      enabled: rootSelectionModel.currentIndex.valid &&
          !model.isFinalBlend(rootSelectionModel.currentIndex)
      symbol: Icons.faTrashAlt
      onClicked: model.removeRowAtIndex(rootSelectionModel.currentIndex)
    }
  }

  ToolOptionsForm {
    Layout.fillWidth: true
    Layout.minimumHeight: 30
    Layout.preferredHeight: 200

    rootModel: model
    selectionModel: rootSelectionModel
  }
}
