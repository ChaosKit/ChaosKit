import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

Item {
  id: root

  required property var blend
  property string name
  property string icon: 'blend'
  property bool open: false
  property bool allowDelete: false
  property alias selectedItem: contents.selectedItem

  signal selected()
  signal childSelected(var child)
  signal deleteRequested()

  implicitHeight: header.height + (open ? contents.height : 0)

  Rectangle {
    id: header
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    color: Theme.controlColor(
        hoverHandler.hovered, tapHandler.pressed, selectedItem === blend.self)
    height: layout.implicitHeight + Theme.smallPadding * 2

    HoverHandler {
      id: hoverHandler
    }
    TapHandler {
      id: tapHandler
      onTapped: root.selected()
    }

    RowLayout {
      id: layout
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: Theme.smallPadding
      spacing: Theme.smallPadding

      IconButton {
        icon.color: Theme.onSurfaceHigh
        iconName: 'arrow-down'
        innerOffset: 0
        rotation: open ? 0 : -90

        onClicked: () => {
          open = !open;
        }
      }

      Icon {
        name: root.icon
      }

      TextLabel {
        Layout.fillWidth: true
        text: name || blend.name || 'Untitled Blend'
      }

      IconButton {
        iconName: 'trash'
        innerOffset: 0
        visible: allowDelete &&
            (hoverHandler.hovered || selectedItem === blend.self)

        onClicked: () => {
          root.deleteRequested();
        }
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

    onChildClicked: (child) => {
      root.childSelected(child);
    }
    onDeleteRequested: (transform) => {
      if (root.selectedItem === transform) {
        root.selected();
      }

      if (transform === blend.pre) {
        blend.removePre();
      } else if (transform === blend.post) {
        blend.removePost();
      }
    }
  }
}
