import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

GridLayout {
  id: root

  required property var system
  // Remember the last max lifetime for when you switch it to infinite.
  property int localMaxLifetime: 20

  columns: 3
  columnSpacing: Theme.smallPadding
  rowSpacing: Theme.smallPadding

  function updateLocalState() {
    if (!system.isInfiniteLifetime) {
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
    to: root.system.isInfiniteLifetime ? 2147483647 : root.localMaxLifetime
    value: root.system.minLifetime

    onValueModified: {
      root.system.minLifetime = value;
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
    checked: !root.system.isInfiniteLifetime

    onClicked: {
      root.system.maxLifetime = root.localMaxLifetime;
    }
  }
  SpinBox {
    id: maxLifetimeField

    Layout.fillWidth: true
    editable: true
    from: root.system.isInfiniteLifetime ? 0 : root.system.minLifetime
    to: 2147483647
    value: root.localMaxLifetime

    onValueModified: {
      root.system.maxLifetime = value;
    }
  }

  RadioButton {
    id: infiniteRadio

    checked: root.system.isInfiniteLifetime
    text: 'Infinite'
    Layout.column: 1
    Layout.columnSpan: 2
    Layout.leftMargin: 2
    Layout.row: 3

    onClicked: {
      root.system.isInfiniteLifetime = true;
    }
  }
}
