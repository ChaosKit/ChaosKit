import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  spacing: Theme.padding

  readonly property var system: documentModel.systemProxy

  RowLayout {
    spacing: Theme.padding

    Label {
      Layout.leftMargin: Theme.padding
      text: "Lifetime"
    }
    GridLayout {
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      columns: 2
      columnSpacing: Theme.smallPadding
      rowSpacing: Theme.padding

      ButtonGroup {
        buttons: [finiteRadio, infiniteRadio]
      }

      RadioButton {
        id: finiteRadio

        checked: system.ttl >= 0
        onClicked: {
          system.ttl = parseInt(ttlField.text, 10);
        }
      }
      TextField {
        Layout.fillWidth: true

        id: ttlField
        text: system.ttl < 0 ? '20' : `${system.ttl}`;
        validator: IntValidator { bottom: 1 }

        onEditingFinished: {
          finiteRadio.checked = true;
          system.ttl = parseInt(text, 10);
        }
      }

      RadioButton {
        Layout.columnSpan: 2
        Layout.leftMargin: 2 // pixel-pushing

        id: infiniteRadio
        checked: system.ttl < 0
        text: 'Infinite'

        onClicked: {
          system.ttl = -1;
        }
      }
    }
  }
}
