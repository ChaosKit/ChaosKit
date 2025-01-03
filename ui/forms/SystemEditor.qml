import QtQml.Models
import QtQuick
import QtQuick.Controls
import ChaosKit

Item {
  id: root
  property var selectedPath: [projectModel.system]

  function getSelectedLeaf() {
    return root.selectedPath[root.selectedPath.length - 1];
  }

  Connections {
    target: projectModel

    // Reset the selection when we reset the project.
    function onProjectChanged() {
      root.selectedPath = [projectModel.system];
    }
  }

  ScrollView {
    anchors.bottom: separator.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    clip: true

    StructureEditor {
      selectedPath: root.selectedPath
      system: projectModel.system
      width: parent.width

      onSelectionChanged: (path) => {
        root.selectedPath = path;
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

  Component {
    id: blendSettings
    BlendSettings {
      blend: getSelectedLeaf()
      isCamera: getSelectedLeaf() === projectModel.system.cameraBlend
    }
  }

  Component {
    id: transformSettings
    TransformSettings {
      transformModel: getSelectedLeaf()
    }
  }

  Component {
    id: formulaSettings
    FormulaSettings {
      blend: selectedPath[1]
    }
  }

  function getSourceComponent() {
    if (getSelectedLeaf() != null) {
      switch (getSelectedLeaf().objectName) {
        case 'blend':
          return blendSettings;
        case 'transform':
          return transformSettings;
        case 'formulas':
          return formulaSettings;
      }
    }

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

