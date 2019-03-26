import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Pane {
  property var rootModel: null
  property var selectionModel: null

  Loader {
    id: loader
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    source: "DocumentOptionsForm.qml"
  }

  Connections {
    target: selectionModel
    onCurrentChanged: {
      if (!current.valid) {
        loader.setSource(
            'DocumentOptionsForm.qml',
            {system: rootModel.modelAtIndex(current)});
      } else if (current.parent.valid) {
        loader.setSource(
            'FormulaOptionsForm.qml',
            {formula: rootModel.modelAtIndex(current)});
      } else {
        loader.setSource(
            'BlendOptionsForm.qml', {
              blend: rootModel.modelAtIndex(current),
              isFinalBlend: rootModel.isFinalBlend(current)});
      }
    }
  }
}
