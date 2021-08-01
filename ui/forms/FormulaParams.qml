import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

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
