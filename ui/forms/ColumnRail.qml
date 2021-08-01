import QtQuick
import ChaosKit

// An item that, if positioned in a column, forms a rail of connected inner
// items

Item {
  id: rail

  property bool isFirstItem: false
  property bool isLastItem: false
  property real topPadding: 0
  property real bottomPadding: 0

  Rectangle {
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.top: parent.top
    color: Theme.onSurfaceMedium
    width: 1
    height: rail.topPadding
    visible: !rail.isFirstItem
  }

  Rectangle {
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    color: Theme.onSurfaceMedium
    width: 1
    height: rail.bottomPadding
    visible: !rail.isLastItem
  }
}
