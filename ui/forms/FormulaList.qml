import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

Column {
  id: root

  required property var blend

  Repeater {
    id: repeater
    model: blend.formulas

    delegate: FormulaItem {
      required property var model
      required property int index

      formula: model.self
      width: parent.width

      onDeleteClicked: {
        blend.deleteFormulaAt(index);
      }
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
      text: 'Add formula'

      onClicked: {
        formulaPopup.open();
      }

      FormulaPopup {
        id: formulaPopup

        onFormulaPicked: {
          blend.addFormula(formulaType);
        }
      }
    }
  }
}
