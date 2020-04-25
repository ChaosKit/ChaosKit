import QtQuick 2.12
import QtQuick.Controls 2.12
import ChaosKit 1.0

Rectangle {
  id: item

  property real weight

  border.color: Theme.onSurfaceMedium
  border.width: 1
  color:
      Theme.blend(
          Theme.surfaceColor,
          Theme.onSurface,
          (mouseArea.containsMouse || dragHandler.active)
              ? Theme.hoverRatio : 0
      )
  height: Theme.blendIndicatorHeight
  radius: Theme.borderRadius
  width: Theme.blendIndicatorWidth

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    cursorShape: Qt.SplitHCursor
    hoverEnabled: true
  }

  DragHandler {
    id: dragHandler
    target: null

    property real startWeight: item.weight

    onActiveChanged: {
      if (active) {
        startWeight = item.weight;
      }
    }
    onTranslationChanged: {
      item.weight = Math.max(startWeight + translation.x * .01, 0);
    }
  }

  Label {
    anchors.fill: parent
    anchors.margins: 1
    font.pointSize: 8
    font.weight: Font.DemiBold
    horizontalAlignment: Text.AlignHCenter
    text: Math.round(weight * 100) / 100
    verticalAlignment: Text.AlignVCenter
  }
}
