import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import ChaosKit 1.0

T.TabBar {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          contentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           contentHeight + topPadding + bottomPadding)

  spacing: 0

  contentItem: ListView {
    model: control.contentModel
    currentIndex: control.currentIndex

    spacing: control.spacing
    orientation: ListView.Horizontal
    boundsBehavior: Flickable.StopAtBounds
    flickableDirection: Flickable.AutoFlickIfNeeded
    snapMode: ListView.SnapToItem

    highlightMoveDuration: 0
    highlightRangeMode: ListView.ApplyRange
    preferredHighlightBegin: 40
    preferredHighlightEnd: width - 40
  }

  background: Rectangle {
    color: "transparent"

    Rectangle {
      id: indicator
      anchors.bottom: parent.bottom
      color: Theme.primaryColor
      height: 2
      x: control.currentItem.x
      width: control.currentItem.width

      Behavior on x {
        NumberAnimation {
          duration: 150
          easing.type: Easing.Bezier
          easing.bezierCurve: Theme.standardCurve
        }
      }
      Behavior on width {
        NumberAnimation {
          duration: 150
          easing.type: Easing.Bezier
          easing.bezierCurve: Theme.standardCurve
        }
      }
    }
  }
}
