import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.12

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

  ToolOptionsForm {
    Layout.fillWidth: true
    Layout.minimumHeight: 30
    Layout.preferredHeight: 200

    rootModel: repeater.model
    selectionModel: rootSelectionModel
  }
}
