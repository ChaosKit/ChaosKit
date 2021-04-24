import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

GridLayout {
  id: root

  required property var system
  // Remember the last max lifetime for when you switch it to infinite.
  property int localMaxLifetime: 20

  columns: 3
  columnSpacing: Theme.smallPadding
  rowSpacing: Theme.smallPadding

  function updateLocalState() {
    if (!system.isImmortal) {
      localMaxLifetime = system.maxLifetime;
    }
  }

  // These two keep localMaxLifetime in sync with the system.
  Component.onCompleted: {
    updateLocalState();
  }
  Connections {
    target: system
    function onMaxLifetimeChanged() {
      updateLocalState();
    }
  }

  Subheading {
    Layout.columnSpan: 3
    text: 'Lifetime'
  }

  TextLabel {
    Layout.rightMargin: Theme.smallPadding
    text: 'Min'
  }
  TextField {
    Layout.columnSpan: 2
    Layout.fillWidth: true
    text: system.minLifetime
    validator: IntValidator {
      bottom: 0
      top: system.isImmortal ? 2147483647 : system.maxLifetime
    }

    onEditingFinished: {
      system.minLifetime = parseInt(text, 10);
    }
  }

  TextLabel {
    Layout.rightMargin: Theme.smallPadding
    text: 'Max'
  }
  ButtonGroup {
    buttons: [finiteRadio, infiniteRadio]
  }
  RadioButton {
    id: finiteRadio
    checked: !system.isImmortal

    onClicked: {
      system.maxLifetime = localMaxLifetime;
    }
  }
  TextField {
    id: maxLifetimeField

    Layout.fillWidth: true
    text: `${localMaxLifetime}`;
    validator: IntValidator { bottom: system.minLifetime }

    onEditingFinished: {
      system.maxLifetime = parseInt(text, 10);
    }
  }

  RadioButton {
    id: infiniteRadio

    Layout.column: 1
    Layout.columnSpan: 2
    Layout.leftMargin: 2
    Layout.row: 3
    checked: system.isImmortal
    text: 'Infinite'

    onClicked: {
      system.isImmortal = true;
    }
  }
}
