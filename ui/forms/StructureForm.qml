import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.12

ColumnLayout {
  id: root
  spacing: 0

  property alias model: listView.model

  ItemSelectionModel {
    id: rootSelectionModel
    model: listView.model
  }

  ListView {
    id: listView
    Layout.fillWidth: true
    Layout.fillHeight: true

    delegate: BlendListItem {
      anchors.left: parent.left
      anchors.right: parent.right
      rootModel: listView.model
      selectionModel: rootSelectionModel
    }
    currentIndex: -1
  }

  ToolOptionsForm {
    Layout.fillWidth: true
    Layout.minimumHeight: 30
    Layout.preferredHeight: 200

    rootModel: listView.model
    selectionModel: rootSelectionModel
  }
}
