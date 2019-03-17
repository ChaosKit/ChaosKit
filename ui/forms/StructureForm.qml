import QtQuick 2.11

ListView {
  id: rootListView
  delegate: BlendListItem {
    anchors.left: parent.left
    anchors.right: parent.right
    rootModel: rootListView.model
  }
}
