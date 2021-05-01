import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

ScrollView {
  id: root

  required property var transformModel

  property bool scaleLocked: false

  clip: true
  contentWidth: availableWidth

  ColumnLayout {
    spacing: 0
    width: parent.width

    Heading {
      Layout.leftMargin: Theme.padding
      Layout.topMargin: Theme.padding
      text: 'Transform Settings'
    }

    GridLayout {
      Layout.fillWidth: true
      Layout.leftMargin: Theme.padding
      Layout.rightMargin: Theme.padding
      Layout.topMargin: Theme.padding
      columns: 7
      columnSpacing: 0
      rowSpacing: Theme.padding

      component NumberField : TextField {
        property real value
        property int precision: 2

        readonly property real factor: Math.pow(10, precision)

        text: Math.round(value * factor) / factor
        validator: DoubleValidator {
          locale: 'en_US'
        }
      }

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
        value: transformModel.translationX

        onEditingFinished: {
          transformModel.translationX = Number(text);
        }
      }
      NumberField {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        value: transformModel.translationY

        onEditingFinished: {
          transformModel.translationY = Number(text);
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
        value: transformModel.scaleX

        onEditingFinished: {
          if (root.scaleLocked) {
            const aspectRatio = transformModel.scaleY / transformModel.scaleX;
            transformModel.scaleY = Number(text) * aspectRatio;
          }
          transformModel.scaleX = Number(text);
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
        value: transformModel.scaleY

        onEditingFinished: {
          if (root.scaleLocked) {
            const aspectRatio = transformModel.scaleX / transformModel.scaleY;
            transformModel.scaleX = Number(text) * aspectRatio;
          }
          transformModel.scaleY = Number(text);
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
        precision: 4
        value: transformModel.rotation

        onEditingFinished: {
          transformModel.rotation = Number(text);
        }
      }
    }
  }
}
