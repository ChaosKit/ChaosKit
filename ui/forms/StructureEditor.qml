import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Column {
  id: root

  required property var system
  property var selectedPath
  
  signal selectionChanged(var path)

  width: parent.width

  function getSelectedLeaf() {
    return root.selectedPath[root.selectedPath.length - 1];
  }

  Rectangle {
    id: header
    anchors.left: parent.left
    anchors.right: parent.right
    color: Theme.controlColor(
        hoverHandler.hovered, tapHandler.pressed, getSelectedLeaf() === system)
    height: layout.implicitHeight + Theme.smallPadding * 2

    HoverHandler {
      id: hoverHandler
    }
    TapHandler {
      id: tapHandler
      onTapped: root.selectionChanged([system])
    }

    RowLayout {
      id: layout
      width: parent.width
      spacing: Theme.smallPadding
      y: Theme.smallPadding

      Icon {
        Layout.leftMargin: Theme.smallPadding
        name: 'solar-system'
      }

      TextLabel {
        Layout.fillWidth: true
        text: 'System'
      }
    }
  }

  Repeater {
    model: system.blends
    delegate: BlendItem {
      required property var model
      required property int index

      allowDelete: true
      blend: model.self
      selectedItem: getSelectedLeaf()
      width: parent.width

      onSelected: {
        root.selectionChanged([system, model.self]);
      }
      onChildSelected: {
        root.selectionChanged([system, model.self, child]);
      }
      onDeleteRequested: {
        system.deleteBlendAt(index);
      }
    }
  }

  BlendItem {
    blend: system.cameraBlend
    icon: 'camera-video'
    name: 'Camera'
    selectedItem: getSelectedLeaf()
    width: parent.width

    onSelected: {
      root.selectionChanged([system, system.cameraBlend]);
    }
    onChildSelected: {
      root.selectionChanged([system, system.cameraBlend, child]);
    }
  }

  Item {
    anchors.left: parent.left
    anchors.right: parent.right
    height: addBlend.implicitHeight + Theme.smallPadding * 2

    ItemPlaceholder {
      id: addBlend
      anchors.fill: parent
      anchors.margins: Theme.smallPadding
      text: 'Add blend'

      onClicked: {
        system.addBlend();
      }
    }
  }
}
