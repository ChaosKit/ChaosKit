import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import ChaosKit 1.0

Item {
  id: root
  property var selectedItem: null

  ScrollView {
    anchors.bottom: separator.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    clip: true

    StructureEditor {
      selectedItem: root.selectedItem
      system: projectModel.system
      width: parent.width

      onSelectionChanged: {
        root.selectedItem = item;
      }
    }
  }

  Separator {
    id: separator
    anchors.left: parent.left
    anchors.right: parent.right
    draggable: true
    minPosition: 100
    y: 200
  }

  Component {
    id: systemSettings
    SystemSettings {
      system: projectModel.system
    }
  }

  function getSourceComponent() {
    return systemSettings;
  }

  Loader {
    id: settings
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: separator.bottom
    sourceComponent: { getSourceComponent() }
  }
}

