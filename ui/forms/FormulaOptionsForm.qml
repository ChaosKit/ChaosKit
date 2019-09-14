import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

GridLayout {
  property var formula

  anchors.margins: 4
  columns: 3
  columnSpacing: 4
  rowSpacing: 0

  Label {
    text: "Weight"
  }
  Slider {
    Layout.fillWidth: true
    from: 0
    to: 2
    stepSize: 0.01
    value: formula.weight

    onMoved: formula.weight = value
  }
  TextField {
    text: formula.weight.toFixed(2)
    validator: DoubleValidator {
      decimals: 2
      locale: "C"
    }

    onAccepted: formula.weight = parseFloat(text)
  }

  Button {
    Layout.columnSpan: 3
    Layout.fillWidth: true

    text: "Randomize Params"
    onClicked: {
      documentModel.randomizeParams(selectionModel.currentIndex);
    }
  }
}
