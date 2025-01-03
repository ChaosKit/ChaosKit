import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

Rectangle {
  id: root

  required property var formulas
  property int offset: 0
  property bool selected: false

  signal clicked()

  color: Theme.controlColor(hoverHandler.hovered, tapHandler.pressed, selected)
  implicitWidth: 150
  implicitHeight: layout.implicitHeight + Theme.smallPadding * 2

  HoverHandler {
    id: hoverHandler
  }
  TapHandler {
    id: tapHandler
    onTapped: root.clicked()
  }

  RowLayout {
    id: layout
    anchors.left: parent.left
    anchors.leftMargin: offset
    anchors.right: parent.right
    spacing: Theme.smallPadding
    y: Theme.smallPadding

    Icon {
      Layout.alignment: Qt.AlignTop
      Layout.leftMargin: Theme.smallPadding
      name: 'formula'
    }

    TextLabel {
      Layout.fillWidth: true
      Layout.rightMargin: Theme.smallPadding
      color: Theme.onSurfaceMedium
      text: 'Formulas'
      visible: formulas.count === 0
    }

    Flow {
      Layout.fillWidth: true
      Layout.rightMargin: Theme.smallPadding
      spacing: Theme.tinyPadding
      visible: formulas.count > 0

      Repeater {
        model: formulas
        delegate: Chip {
          text: type
        }
      }
    }
  }
}
