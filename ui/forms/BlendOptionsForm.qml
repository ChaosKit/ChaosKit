import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import app.chaoskit 1.0

GridLayout {
  property var blend

  anchors.margins: 4
  columns: 2
  columnSpacing: 4
  rowSpacing: 0

  Component.onCompleted: {
    const formulaIndex = blend.singleFormulaIndex;
    if (!formulaIndex) return;

    formulaFormLoader.setSource('FormulaOptionsForm.qml', {
      formula: documentModel.entryAtIndex(formulaIndex)
    });
  }

  Label {
    text: "Name"
    visible: blend.type === DocumentEntryType.Blend
  }
  TextField {
    placeholderText: blend.display
    text: blend.edit || ""
    visible: blend.type === DocumentEntryType.Blend

    onTextEdited: blend.edit = text
  }

  Loader {
    id: formulaFormLoader
    Layout.columnSpan: 2
    Layout.fillWidth: true
  }
}
