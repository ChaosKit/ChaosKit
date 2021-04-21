import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

Item {
  id: root

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
        icon.color: Theme.onSurfaceHigh
        iconName: 'arrow-down'
        rotation: open ? 0 : -90

        onClicked: toggleOpen()
      }

      Label {
        Layout.fillWidth: true
        Layout.rightMargin: Theme.smallPadding
        text: name || 'Untitled Blend'
      }
    }
  }

  Rectangle {
    id: contents
    anchors.top: header.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    height: 70
    visible: open
  }

  
}