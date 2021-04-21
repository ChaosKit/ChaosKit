import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

Item {
  id: root

  property var blend
  property string name
  property bool open: false

  implicitHeight: header.height + (open ? contents.height : 0)

  function toggleOpen() {
    open = !open;
  }

  Rectangle {
    id: header
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    color: Theme.controlColor(hoverHandler.hovered, tapHandler.pressed)
    height: layout.implicitHeight + Theme.smallPadding * 2

    HoverHandler {
      id: hoverHandler
    }
    TapHandler {
      id: tapHandler
      onTapped: toggleOpen()
    }

    RowLayout {
      id: layout
      width: parent.width
      spacing: Theme.smallPadding
      y: Theme.smallPadding

      Icon {
        Layout.leftMargin: Theme.smallPadding
        tint: Theme.onSurfaceHigh
        name: 'arrow-down'
        rotation: open ? 0 : -90
      }

      Label {
        Layout.fillWidth: true
        Layout.rightMargin: Theme.smallPadding
        text: name || blend.name || 'Untitled Blend'
      }
    }
  }

  BlendContents {
    id: contents
    anchors.top: header.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    blend: root.blend
    offset: Theme.smallIconSize + Theme.smallPadding
    visible: open
  }
}
