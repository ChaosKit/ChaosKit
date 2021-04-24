import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Column {
  id: root

  required property var system
  property var selectedItem
  
  signal selectionChanged(var item)

  width: parent.width

  Repeater {
    model: system.blends
    delegate: BlendItem {
      required property var model

      blend: model.self
      selectedItem: root.selectedItem
      width: parent.width

      onClicked: {
        root.selectionChanged(model.self);
      }
      onChildClicked: {
        root.selectionChanged(child);
      }
    }
  }

  BlendItem {
    blend: system.cameraBlend
    icon: 'camera-video'
    name: 'Camera'
    selectedItem: root.selectedItem
    width: parent.width

    onClicked: {
      root.selectionChanged(system.cameraBlend);
    }
    onChildClicked: {
      root.selectionChanged(child);
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
