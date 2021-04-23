import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

Rectangle {
  id: root

  property string name
  property int offset: 0
  property var transformModel

  color: Theme.controlColor(hoverHandler.hovered, tapHandler.pressed)
  implicitWidth: 150
  implicitHeight: Theme.smallIconSize + Theme.smallPadding * 2

  HoverHandler {
    id: hoverHandler
  }
  TapHandler {
    id: tapHandler
  }

  RowLayout {
    id: layout
    anchors.fill: parent
    anchors.leftMargin: offset
    spacing: Theme.smallPadding
    y: Theme.smallPadding

    Icon {
      Layout.leftMargin: Theme.smallPadding
      name: 'transform'
    }

    Label {
      Layout.fillWidth: true
      Layout.rightMargin: Theme.smallPadding
      text: root.name
    }
  }
}
