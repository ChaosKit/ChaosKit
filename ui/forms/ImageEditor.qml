import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

GridLayout {
  readonly property var document: documentModel.documentProxy

  columns: 3

  Label {
    Layout.columnSpan: 2
    Layout.fillWidth: true

    color: Theme.onSurfaceMedium
    font.pointSize: 14
    font.letterSpacing: Theme.letterSpacing(14)
    font.weight: Font.DemiBold
    padding: Theme.units(2)
    text: "Tone Mapping"
  }
  IconButton {
    Layout.rightMargin: Theme.units(2)
    iconName: 'undo'
    size: 10

    onClicked: {
      gammaSlider.value = 2.2;
      exposureSlider.value = 0.0;
      vibrancySlider.value = 0.0;
    }
  }

  // Gamma

  Label {
    Layout.fillWidth: true
    leftPadding: Theme.units(2)
    text: "Gamma"
  }
  Label {
    Layout.alignment: Qt.AlignRight
    horizontalAlignment: Text.AlignRight
    text: document.gamma.toFixed(2)
  }
  IconButton {
    Layout.rightMargin: Theme.units(2)
    iconName: 'undo'
    size: 10

    onClicked: {
      gammaSlider.value = 2.2;
    }
  }
  Slider {
    id: gammaSlider
    Layout.bottomMargin: Theme.units(2)
    Layout.columnSpan: 3
    Layout.fillWidth: true

    from: 0
    leftPadding: Theme.units(2)
    rightPadding: Theme.units(2)
    to: 10
    value: 2.2
  }
  Binding {
    target: document
    property: 'gamma'
    value: gammaSlider.value
  }

  // Exposure

  Label {
    Layout.fillWidth: true
    leftPadding: Theme.units(2)
    text: "Exposure"
  }
  Label {
    Layout.alignment: Qt.AlignRight
    horizontalAlignment: Text.AlignRight
    text: document.exposure.toFixed(2)
  }
  IconButton {
    Layout.rightMargin: Theme.units(2)
    iconName: 'undo'
    size: 10

    onClicked: {
      exposureSlider.value = 0.0;
    }
  }
  Slider {
    id: exposureSlider

    Layout.bottomMargin: Theme.units(2)
    Layout.columnSpan: 3
    Layout.fillWidth: true

    from: -1
    leftPadding: Theme.units(2)
    rightPadding: Theme.units(2)
    to: 2
    value: 0
  }
  Binding {
    target: document
    property: 'exposure'
    value: exposureSlider.value
  }

  // Vibrancy

  Label {
    Layout.fillWidth: true
    leftPadding: Theme.units(2)
    text: "Vibrancy"
  }
  Label {
    Layout.alignment: Qt.AlignRight
    horizontalAlignment: Text.AlignRight
    text: document.vibrancy.toFixed(2)
  }
  IconButton {
    Layout.rightMargin: Theme.units(2)
    iconName: 'undo'
    size: 10

    onClicked: {
      vibrancySlider.value = 0.0;
    }
  }
  Slider {
    id: vibrancySlider

    Layout.columnSpan: 3
    Layout.fillWidth: true

    from: 0
    leftPadding: Theme.units(2)
    rightPadding: Theme.units(2)
    to: 1
    value: 0
  }
  Binding {
    target: document
    property: 'vibrancy'
    value: vibrancySlider.value
  }
}
