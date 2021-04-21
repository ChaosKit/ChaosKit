import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Column {
  id: column
  width: parent.width

  Repeater {
    model: projectModel.system.blends
    delegate: BlendItem {
      blend: model.self
      width: parent.width
    }
  }

  BlendItem {
    blend: projectModel.system.cameraBlend
    name: 'Camera'
    width: parent.width
  }

  ItemPlaceholder {
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: Theme.smallPadding
    text: 'Add blend'

    onClicked: {
      projectModel.system.addBlend();
    }
  }
}
