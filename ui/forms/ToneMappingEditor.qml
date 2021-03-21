import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

GridLayout {
  columns: 3
  columnSpacing: Theme.padding
  rowSpacing: Theme.smallPadding

  Heading {
    Layout.columnSpan: 2
    Layout.fillWidth: true

    padding: Theme.padding
    text: "Tone Mapping"
  }
  IconButton {
    Layout.rightMargin: Theme.padding
    iconName: 'undo'
    size: Theme.smallIconSize

    onClicked: {
      projectModel.gamma = 2.2;
      projectModel.exposure = 0.0;
      projectModel.vibrancy = 0.0;
    }
  }

  // Gamma

  Label {
    leftPadding: Theme.padding
    text: "Gamma"
  }
  Label {
    Layout.alignment: Qt.AlignRight
    Layout.fillWidth: true
    horizontalAlignment: Text.AlignRight
    text: projectModel.gamma.toFixed(2)
  }
  IconButton {
    Layout.rightMargin: Theme.padding
    iconName: 'undo'
    size: Theme.smallIconSize

    onClicked: {
      projectModel.gamma = 2.2;
    }
  }
  Slider {
    id: gammaSlider
    Layout.columnSpan: 3
    Layout.fillWidth: true

    from: 0
    leftPadding: Theme.padding
    rightPadding: Theme.padding
    to: 10
    value: projectModel.gamma

    onMoved: {
      projectModel.gamma = value;
    }
  }

  // Exposure

  Label {
    leftPadding: Theme.padding
    text: "Exposure"
  }
  Label {
    Layout.alignment: Qt.AlignRight
    Layout.fillWidth: true
    horizontalAlignment: Text.AlignRight
    text: projectModel.exposure.toFixed(2)
  }
  IconButton {
    Layout.rightMargin: Theme.padding
    iconName: 'undo'
    size: Theme.smallIconSize

    onClicked: {
      projectModel.exposure = 0.0;
    }
  }
  Slider {
    id: exposureSlider

    Layout.columnSpan: 3
    Layout.fillWidth: true

    from: -1
    leftPadding: Theme.padding
    rightPadding: Theme.padding
    to: 2
    value: projectModel.exposure

    onMoved: {
      projectModel.exposure = value;
    }
  }

  // Vibrancy

  Label {
    leftPadding: Theme.padding
    text: "Vibrancy"
  }
  Label {
    Layout.alignment: Qt.AlignRight
    Layout.fillWidth: true
    horizontalAlignment: Text.AlignRight
    text: projectModel.vibrancy.toFixed(2)
  }
  IconButton {
    Layout.rightMargin: Theme.padding
    iconName: 'undo'
    size: Theme.smallIconSize

    onClicked: {
      projectModel.vibrancy = 0.0;
    }
  }
  Slider {
    id: vibrancySlider

    Layout.columnSpan: 3
    Layout.fillWidth: true

    from: 0
    leftPadding: Theme.padding
    rightPadding: Theme.padding
    to: 1
    value: projectModel.vibrancy

    onMoved: {
      projectModel.vibrancy = value;
    }
  }
}
