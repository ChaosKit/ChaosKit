import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import app.chaoskit 1.0

Pane {
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
      const entry = documentModel.entryAtIndex(current);

      switch (entry.type) {
        case DocumentEntryType.Formula:
          loader.setSource('FormulaOptionsForm.qml', {formula: entry});
          break;
        case DocumentEntryType.Blend:
        case DocumentEntryType.FinalBlend:
          loader.setSource('BlendOptionsForm.qml', {blend: entry});
          break;
        default:
          loader.setSource('DocumentOptionsForm.qml', {});
      }
    }
  }
}
