import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

Item {
  id: root

  required property var formula
  property bool open: false

  signal deleteClicked()

  implicitWidth: 150
  implicitHeight: {layout.implicitHeight + Theme.smallPadding * 2 + (open ? params.height + Theme.padding : 0)}

  RowLayout {
    id: layout
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.margins: Theme.smallPadding
    spacing: Theme.smallPadding

    IconButton {
      icon.color: Theme.onSurfaceHigh
      iconName: 'arrow-down'
      rotation: open ? 0 : -90
      visible: formula.params.length > 0

      onClicked: {
        open = !open;
      }
    }

    TextLabel {
      Layout.leftMargin: formula.params.length > 0 ? 0 : Theme.padding
      Layout.fillWidth: true
      text: formula.type
    }

    NumberField {
      Layout.preferredWidth: 90
      from: 0
      stepSize: 0.1 * factor
      to: 2147483647
      value: formula.weight * factor

      onValueModified: {
        formula.weight = realValue;
      }
    }

    IconButton {
      iconName: 'trash'

      onClicked: {
        root.deleteClicked();
      }
    }
  }

  FormulaParams {
    id: params
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: layout.bottom
    anchors.margins: Theme.padding
    formula: root.formula
    visible: open
  }
}
