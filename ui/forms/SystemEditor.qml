import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  id: root
  spacing: 0

  property var currentBlend:
      documentModel.entryAtIndex(selectionModel.currentIndex)

  StructureEditor {
    Layout.fillWidth: true
  }
  FormulaGallery {
    dragContainer: root
    Layout.fillWidth: true
  }
  LifetimeEditor {
    Layout.fillWidth: true
  }

  // Blend editor

  CollapsibleHeading {
    id: heading
    opened: true
    text: 'Selected Blend'
  }
  Label {
    color: Theme.onSurfaceDisabled
    horizontalAlignment: Text.AlignHCenter
    padding: Theme.padding
    text: 'Nothing selected'
    visible: heading.opened && !selectionModel.currentIndex.valid
    Layout.fillWidth: true
  }
  Component {
    id: blendEditor

    BlendEditor {
      blend: currentBlend
    }
  }
  Loader {
    active: selectionModel.currentIndex.valid
    sourceComponent: blendEditor
    Layout.fillWidth: true
  }
}
