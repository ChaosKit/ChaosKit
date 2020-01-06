import QtQuick 2.12
import QtQuick.Controls 2.12
import ChaosKit 1.0

ApplicationWindow {
  height: 768
  title: "ChaosKit"
  visible: true
  width: 1024

  SystemPreview {
    anchors.centerIn: parent

    Component.onCompleted: {
      running = true;
    }
  }

  Fab {
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: Theme.units(4)
    enabled: true
    iconName: "random"
    ToolTip.text: "Randomize the image"
    ToolTip.visible: hovered

    onClicked: {
      documentModel.randomizeSystem();
    }
  }
}
