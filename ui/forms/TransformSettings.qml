import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

ScrollView {
  id: root

  required property var transformModel

  property bool scaleLocked: false

  clip: true
  contentWidth: availableWidth

  ColumnLayout {
    spacing: 0
    width: parent.width

    RowLayout {
      Layout.fillWidth: true
      Layout.leftMargin: Theme.padding
      Layout.rightMargin: Theme.padding
      Layout.topMargin: Theme.padding

      Heading {
        Layout.fillWidth: true
        text: 'Transform Settings'
      }

      IconButton {
        Layout.columnSpan: 2
        iconName: 'undo'

        onClicked: {
          transformModel.reset();
        }
      }
    }

    GridLayout {
      Layout.fillWidth: true
      Layout.leftMargin: Theme.padding
      Layout.rightMargin: Theme.padding
      Layout.topMargin: Theme.padding
      columns: 7
      columnSpacing: 0
      rowSpacing: Theme.padding

      // Translation
      Icon {
        Layout.rightMargin: Theme.padding
        name: 'move'
        tooltip: 'Translation'
      }
      NumberField {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        Layout.rightMargin: Theme.padding
        from: -100 * factor | 0
        stepSize: 0.1 * factor | 0
        to: 100 * factor | 0
        value: transformModel.translationX * factor

        onValueModified: {
          transformModel.translationX = realValue;
        }
      }
      NumberField {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        from: -100 * factor | 0
        stepSize: 0.1 * factor | 0
        to: 100 * factor | 0
        value: transformModel.translationY * factor

        onValueModified: {
          transformModel.translationY = realValue;
        }
      }

      // Scale
      Icon {
        Layout.rightMargin: Theme.padding
        name: 'scale'
        tooltip: 'Scale'
      }
      NumberField {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        from: -100 * factor | 0
        stepSize: 0.1 * factor | 0
        to: 100 * factor | 0
        value: transformModel.scaleX * factor

        onValueModified: {
          if (root.scaleLocked) {
            const aspectRatio = transformModel.scaleY / transformModel.scaleX;
            transformModel.scaleY = realValue * aspectRatio;
          }
          transformModel.scaleX = realValue;
        }
      }
      IconButton {
        Layout.columnSpan: 2
        iconName: root.scaleLocked ? 'lock' : 'unlock'

        onClicked: {
          root.scaleLocked = !root.scaleLocked;
        }
      }
      NumberField {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        from: -100 * factor | 0
        stepSize: 0.1 * factor | 0
        to: 100 * factor | 0
        value: transformModel.scaleY * factor

        onValueModified: {
          if (root.scaleLocked) {
            const aspectRatio = transformModel.scaleX / transformModel.scaleY;
            transformModel.scaleX = realValue * aspectRatio;
          }
          transformModel.scaleY = realValue;
        }
      }

      // Rotation
      Icon {
        Layout.rightMargin: Theme.padding
        name: 'rotate'
        tooltip: 'Rotation'
      }
      NumberField {
        Layout.columnSpan: 6
        Layout.fillWidth: true
        from: 0
        precision: 0
        stepSize: 1
        to: 359
        value: transformModel.rotation
        wrap: true

        onValueModified: {
          transformModel.rotation = realValue;
        }
      }
    }
  }
}
