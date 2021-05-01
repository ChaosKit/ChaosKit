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
  SpinBox {
    Layout.columnSpan: 2
    Layout.fillWidth: true
    editable: true
    from: 0
    to: system.isImmortal ? 2147483647 : localMaxLifetime
    value: system.minLifetime

    onValueModified: {
      system.minLifetime = value;
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
  SpinBox {
    id: maxLifetimeField

    Layout.fillWidth: true
    editable: true
    from: system.isImmortal ? 0 : system.minLifetime
    to: 2147483647
    value: localMaxLifetime

    onValueModified: {
      system.maxLifetime = value;
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
