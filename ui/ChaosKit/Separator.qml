import QtQuick 2.12

Rectangle {
  enum Direction {
    Horizontal,
    Vertical
  }

  property int direction: Separator.Direction.Horizontal

  color: Theme.borderColor
  implicitHeight: direction === Separator.Direction.Horizontal ? 1 : parent.height
  implicitWidth: direction === Separator.Direction.Horizontal ? parent.width : 1
}
