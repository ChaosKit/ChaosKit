import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

ScrollView {
  id: root

  required property var blend
  property bool isCamera: false

  clip: true
  contentWidth: availableWidth

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
      columnSpacing: Theme.smallPadding
      rowSpacing: Theme.smallPadding

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
    }
  }
}
