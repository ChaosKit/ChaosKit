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

  Button {
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.leftMargin: Theme.units(2)
    anchors.bottomMargin: Theme.units(2)
    text: "AST Inspector"

    onClicked: {
      astInspectorLoader.active = true;
    }

    Loader {
      id: astInspectorLoader
      active: false
      sourceComponent: AstInspector {
        anchors.centerIn: Overlay.overlay
        source: documentModel.debugSource

        onClosed: {
          astInspectorLoader.active = false;
        }
      }
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
