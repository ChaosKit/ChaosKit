import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

GridLayout {
  readonly property var system: documentModel.systemProxy

  columns: 2
  columnSpacing: Theme.smallPadding
  rowSpacing: 0

  CollapsibleHeading {
    id: heading
    text: "Lifetime"
    Layout.columnSpan: 2
  }

  ButtonGroup {
    buttons: [finiteRadio, infiniteRadio]
  }

  RadioButton {
    id: finiteRadio
    checked: system.ttl >= 0
    visible: heading.opened

    Layout.leftMargin: Theme.padding

    onClicked: {
      system.ttl = parseInt(ttlField.text, 10);
    }
  }
  TextField {
    id: ttlField
    text: system.ttl < 0 ? '20' : `${system.ttl}`;
    validator: IntValidator { bottom: 1 }
    visible: heading.opened

    Layout.fillWidth: true
    Layout.rightMargin: Theme.padding

    onEditingFinished: {
      finiteRadio.checked = true;
      system.ttl = parseInt(text, 10);
    }
  }

  RadioButton {
    id: infiniteRadio
    checked: system.ttl < 0
    text: 'Infinite'
    visible: heading.opened

    Layout.bottomMargin: Theme.padding
    Layout.columnSpan: 2
    Layout.leftMargin: Theme.padding + 2  // pixel-pushing
    Layout.topMargin: Theme.padding

    onClicked: {
      system.ttl = -1;
    }
  }

  RowLayout {
    spacing: Theme.padding
    visible: heading.opened
    Layout.columnSpan: 2
    Layout.fillWidth: true
    Layout.leftMargin: Theme.padding
    Layout.rightMargin: Theme.padding
    Layout.bottomMargin: Theme.padding

    Label {
      text: 'Skip first'
    }
    TextField {
      text: system.skip
      validator: IntValidator {
        bottom: 0
        top: system.ttl < 0 ? 2147483647 : system.ttl
      }
      Layout.fillWidth: true

      onEditingFinished: {
        system.skip = Number(text);
      }
    }
  }
}
