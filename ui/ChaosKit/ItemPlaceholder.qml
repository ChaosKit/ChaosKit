import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
  id: root

  property alias text: label.text

  signal clicked()

  border.color: Theme.borderColor
  border.width: 1
  color: Theme.blend(Theme.surfaceColor, Theme.onSurface, Theme.controlColorValue(hoverHandler.hovered, tapHandler.pressed))
  implicitHeight: Theme.smallIconSize + Theme.smallPadding * 2
  implicitWidth: 150
  radius: Theme.borderRadius

  HoverHandler {
    id: hoverHandler
  }
  TapHandler {
    id: tapHandler
    onTapped: root.clicked()
  }

  RowLayout {
    id: layout
    anchors.fill: parent
    anchors.leftMargin: Theme.smallPadding
    anchors.rightMargin: Theme.smallPadding
    spacing: Theme.smallPadding

    Icon {
      name: 'plus'
      tint: Theme.onSurfaceMedium
    }
    TextLabel {
      id: label
      Layout.fillWidth: true
      color: Theme.onSurfaceMedium
    }
  }
}
