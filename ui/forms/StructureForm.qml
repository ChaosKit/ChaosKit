import QtQuick 2.12
import QtQml.Models 2.12

ListView {
  id: rootListView
  delegate: BlendListItem {
    anchors.left: parent.left
    anchors.right: parent.right
    rootModel: rootListView.model
    selectionModel: rootSelectionModel
  }
  currentIndex: -1

  ItemSelectionModel {
    id: rootSelectionModel
    model: rootListView.model
  }
}
