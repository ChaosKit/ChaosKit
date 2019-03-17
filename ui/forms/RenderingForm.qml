import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

GridLayout {
  columns: 3
  rowSpacing: 4
  columnSpacing: 8

  property alias gamma: gammaSlider.value
  property alias exposure: exposureSlider.value
  property alias vibrancy: vibrancySlider.value

  Label {
    text: "Gamma"
  }
  Slider {
    id: gammaSlider
    value: 2.2
    from: 0
    to: 10
    stepSize: 0.01

    Layout.fillWidth: true
  }
  Label {
    text: gammaSlider.value.toFixed(2)
    horizontalAlignment: Text.AlignRight
    Layout.preferredWidth: 36
  }

  Label {
    text: "Exposure"
  }
  Slider {
    id: exposureSlider
    value: 0
    from: -2
    to: 5
    stepSize: 0.01

    Layout.fillWidth: true
  }
  Label {
    text: exposureSlider.value.toFixed(2)
    horizontalAlignment: Text.AlignRight
    Layout.preferredWidth: 36
  }

  Label {
    text: "Vibrancy"
  }
  Slider {
    id: vibrancySlider
    value: 0
    from: 0
    to: 1
    stepSize: 0.01

    Layout.fillWidth: true
  }
  Label {
    text: vibrancySlider.value.toFixed(2)
    horizontalAlignment: Text.AlignRight
    Layout.preferredWidth: 36
  }
}
