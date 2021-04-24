import QtQuick 2.15

Rectangle {
  id: root

  enum Direction {
    Horizontal,
    Vertical
  }

  property int direction: Separator.Direction.Horizontal
  property bool draggable: false
  property var minPosition: {}
  property var maxPosition: {}

  readonly property bool isHorizontal:
      direction === Separator.Direction.Horizontal

  color: Theme.elevateOpaque(
      Theme.borderRatio +
      Theme.controlColorValue(hoverHandler.hovered, dragHandler.active))
  implicitHeight: isHorizontal ? 1 : parent.height
  implicitWidth: isHorizontal ? parent.width : 1

  Item {
    height: isHorizontal ? 1 + 2 * Theme.tinyPadding : parent.height
    width: isHorizontal ? parent.width : 1 + 2 * Theme.tinyPadding
    visible: draggable
    x: isHorizontal ? 0 : -Theme.tinyPadding
    y: isHorizontal ? -Theme.tinyPadding : 0

    HoverHandler {
      id: hoverHandler
      enabled: draggable
      cursorShape: isHorizontal ? Qt.SizeVerCursor : Qt.SizeHorCursor
    }

    DragHandler {
      id: dragHandler
      cursorShape: isHorizontal ? Qt.SizeVerCursor : Qt.SizeHorCursor
      enabled: draggable
      target: root
      xAxis.enabled: !isHorizontal
      yAxis.enabled: isHorizontal

      Binding on xAxis.minimum {
        when: !isHorizontal && minPosition !== undefined
        value: minPosition
      }
      Binding on xAxis.maximum {
        when: !isHorizontal && maxPosition !== undefined
        value: maxPosition
      }
      Binding on yAxis.minimum {
        when: isHorizontal && minPosition !== undefined
        value: minPosition
      }
      Binding on yAxis.maximum {
        when: isHorizontal && maxPosition !== undefined
        value: maxPosition
      }
    }
  }
}
