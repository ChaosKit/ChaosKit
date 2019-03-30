import QtQuick 2.12
import QtQuick.Layouts 1.12
import "../controls"
import "../resources"

ColumnLayout {
  id: root
  spacing: 0

  property alias model: toolOptions.rootModel
  property alias selectionModel: toolOptions.selectionModel

  Repeater {
    id: repeater
    model: root.model
    delegate: BlendListItem {
      Layout.fillWidth: true
      rootModel: root.model
      selectionModel: root.selectionModel
    }
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
      onClicked: model.addBlend()
    }

    Item {
      Layout.fillWidth: true
    }

    SymbolButton {
      enabled: selectionModel.currentIndex.valid &&
          !model.isFinalBlend(selectionModel.currentIndex)
      symbol: Icons.faTrashAlt
      onClicked: model.removeRowAtIndex(selectionModel.currentIndex)
    }
  }

  ToolOptionsForm {
    id: toolOptions
    Layout.fillWidth: true
    Layout.minimumHeight: 30
    Layout.preferredHeight: 200
  }
}
