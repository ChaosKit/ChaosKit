import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

GridLayout {
  id: root

  required property var formula

  columns: 2
  columnSpacing: Theme.padding
  rowSpacing: Theme.padding

  RowLayout {
    Layout.columnSpan: 2
    Layout.fillWidth: true
    spacing: 0

    Subheading {
      Layout.fillWidth: true
      text: 'Params'
    }

    IconButton {
      iconName: 'random'
      onClicked: {
        formula.randomizeParams();
      }
    }
  }

  Repeater {
    model: formula.params
    delegate: TextField {
      Layout.fillWidth: true
      text: modelData.toFixed(10)
      validator: DoubleValidator {}

      onEditingFinished: {
        formula.setParam(index, Number(text));
      }
    }
  }
}
