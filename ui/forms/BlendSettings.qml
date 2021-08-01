import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

ScrollView {
  id: root

  required property var blend
  property bool isCamera: false

  // The parameter of the coloring method. Updated in real time from the slider
  // and synced periodically with the model for performance.
  property real colorParameter: 0

  // Syncs the parameter value from the model.
  function syncParameterFromBlend() {
    if (root.colorParameter != blend.coloringMethod.parameter) {
      root.colorParameter = blend.coloringMethod.parameter;
    }
  }

  clip: true
  contentWidth: availableWidth

  onBlendChanged: {
    syncParameterFromBlend();
  }

  Component.onCompleted: {
    // Initial sync from the model.
    colorParameter = blend.coloringMethod.parameter;
  }

  Connections {
    target: blend.coloringMethod

    function onParameterChanged() {
      syncParameterFromBlend();
    }
  }

  Timer {
    id: parameterUpdateDebouncer
    interval: 200

    onTriggered: {
      // Store the local value in the model.
      blend.coloringMethod.parameter = root.colorParameter;
    }
  }

  ColumnLayout {
    anchors.top: parent.top
    anchors.topMargin: Theme.padding
    spacing: 0
    width: root.availableWidth

    Heading {
      Layout.leftMargin: Theme.padding
      text: 'Blend Settings'
    }

    GridLayout {
      Layout.fillWidth: true
      Layout.leftMargin: Theme.padding
      Layout.rightMargin: Theme.padding
      Layout.topMargin: Theme.largePadding
      columns: 2
      columnSpacing: Theme.padding
      rowSpacing: Theme.padding

      TextLabel {
        text: 'Enabled'
        visible: !root.isCamera
      }
      Switch {
        Layout.alignment: Qt.AlignRight
        checked: blend.enabled
        visible: !root.isCamera

        onToggled: {
          blend.enabled = checked;
        }
      }

      TextLabel {
        text: 'Weight'
        visible: !root.isCamera
      }
      TextField {
        Layout.fillWidth: true
        text: { blend.weight.toFixed(2) }
        validator: DoubleValidator {
          bottom: 0
        }
        visible: !root.isCamera

        onEditingFinished: {
          blend.weight = parseFloat(text);
        }
      }

      TextLabel {
        text: 'Coloring method'
      }
      ComboBox {
        Layout.fillWidth: true
        currentIndex: blend.coloringMethod.typeIndex
        model: blend.coloringMethod.types

        onActivated: {
          blend.coloringMethod.typeIndex = index;
        }
      }

      Item {
        Layout.fillWidth: true
        height: colorLabel.height

        visible: blend.coloringMethod.typeIndex > 0

        TextLabel {
          id: colorLabel
          text: blend.coloringMethod.typeIndex === 1 ? 'Color' : 'Scale'
        }

        Rectangle {
          anchors.top: colorLabel.top
          anchors.bottom: colorLabel.bottom
          anchors.right: parent.right
          // Using the local parameter here for instant updates
          color: projectModel.colorMap.colorAt(root.colorParameter)
          width: height
          visible: blend.coloringMethod.typeIndex === 1
        }
      }
      Slider {
        Layout.fillWidth: true
        value: root.colorParameter
        visible: blend.coloringMethod.typeIndex > 0

        onMoved: {
          root.colorParameter = value;
          parameterUpdateDebouncer.restart();
        }
      }
    }
  }
}
