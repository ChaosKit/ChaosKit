import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Column {
  id: column
  width: parent.width

  property var selectedItem

  Repeater {
    model: projectModel.system.blends
    delegate: BlendItem {
      blend: model.self
      selectedItem: column.selectedItem
      width: parent.width

      onClicked: {
        column.selectedItem = model.self;
      }
      onChildClicked: {
        column.selectedItem = child;
      }
    }
  }

  BlendItem {
    blend: projectModel.system.cameraBlend
    icon: 'camera-video'
    name: 'Camera'
    selectedItem: column.selectedItem
    width: parent.width

    onClicked: {
      column.selectedItem = projectModel.system.cameraBlend;
    }
    onChildClicked: {
      column.selectedItem = child;
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
        projectModel.system.addBlend();
      }
    }
  }
}
