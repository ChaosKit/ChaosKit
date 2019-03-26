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
    Layout.columnSpan: 3
    text: "Weights"
    font.bold: true
  }

  Label {
    text: "X"
  }
  Slider {
    Layout.fillWidth: true
    from: 0
    to: 2
    stepSize: 0.01
    value: formula.weightX

    onMoved: formula.weightX = value
  }
  TextField {
    text: formula.weightX.toFixed(2)
    validator: DoubleValidator {
      decimals: 2
      locale: "C"
    }

    onAccepted: formula.weightX = parseFloat(text)
  }

  Label {
    text: "Y"
  }
  Slider {
    Layout.fillWidth: true
    from: 0
    to: 2
    stepSize: 0.01
    value: formula.weightY

    onMoved: formula.weightY = value
  }
  TextField {
    text: formula.weightY.toFixed(2)
    validator: DoubleValidator {
      decimals: 2
      locale: "C"
    }
    width: 30
    onAccepted: formula.weightY = parseFloat(text)
  }
}
