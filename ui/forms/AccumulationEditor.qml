// qmllint disable import
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

GridLayout {
  id: root

  property QtObject systemView: null

  // qmllint disable unresolved-type missing-property unqualified
  columns: 2
  columnSpacing: Theme.padding
  rowSpacing: Theme.smallPadding

  Heading {
    padding: Theme.padding
    text: "Accumulation"

    Layout.columnSpan: 2
  }

  Label {
    leftPadding: Theme.padding
    text: "Target IPP"
  }
  SpinBox {
    id: targetIppField

    editable: true
    enabled: root.systemView !== null && !root.systemView.infiniteAccumulation
    from: 1
    stepSize: 100
    to: 2147483647
    value: root.systemView !== null
           ? Math.max(1, Math.round(root.systemView.targetIterationsPerPixel))
           : 1000

    Layout.fillWidth: true
    Layout.rightMargin: Theme.padding

    onValueModified: {
      if (root.systemView !== null) {
        root.systemView.targetIterationsPerPixel = value;
      }
    }
  }

  Label {
    leftPadding: Theme.padding
    text: "Infinite"
  }
  Switch {
    id: infiniteSwitch

    checked: root.systemView !== null && root.systemView.infiniteAccumulation
    enabled: root.systemView !== null

    Layout.alignment: Qt.AlignRight
    Layout.rightMargin: Theme.padding

    onToggled: {
      if (root.systemView !== null) {
        root.systemView.infiniteAccumulation = checked;
      }
    }
  }

  Label {
    leftPadding: Theme.padding
    text: "Current IPP"
  }
  Label {
    id: currentIppLabel

    horizontalAlignment: Text.AlignRight
    text: root.systemView !== null
          ? root.systemView.currentIterationsPerPixel.toFixed(1)
          : "0.0"

    Layout.alignment: Qt.AlignRight
    Layout.fillWidth: true
    Layout.rightMargin: Theme.padding
  }

  Label {
    leftPadding: Theme.padding
    text: "Status"
  }
  Label {
    id: statusLabel

    color: root.systemView !== null && root.systemView.converged
           ? Theme.primaryColor
           : Theme.onSurfaceMedium
    font.weight: root.systemView !== null && root.systemView.converged
                 ? Font.DemiBold
                 : Font.Normal
    horizontalAlignment: Text.AlignRight
    text: root.systemView === null
          ? "Idle"
          : (root.systemView.converged
             ? "Converged"
             : (root.systemView.running ? "Accumulating" : "Paused"))

    Layout.alignment: Qt.AlignRight
    Layout.fillWidth: true
    Layout.rightMargin: Theme.padding
  }
  // qmllint enable unresolved-type missing-property unqualified
}
// qmllint enable import
