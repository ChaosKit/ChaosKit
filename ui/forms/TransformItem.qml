import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

Rectangle {
  id: root

  property string name: 'Transform'
  property int offset: 0
  required property var transformModel
  property bool selected: false

  signal clicked()
  signal deleteClicked()

  color: Theme.controlColor(hoverHandler.hovered, tapHandler.pressed, selected)
  implicitWidth: 150
  implicitHeight: Theme.smallIconSize + Theme.smallPadding * 2

  HoverHandler {
    id: hoverHandler
  }
  TapHandler {
    id: tapHandler
    onTapped: root.clicked()
  }

  RowLayout {
    id: layout
    anchors.fill: parent
    anchors.leftMargin: offset
    anchors.margins: Theme.smallPadding
    spacing: Theme.smallPadding

    Icon {
      name: 'transform'
    }

    TextLabel {
      Layout.fillWidth: true
      text: root.name
    }

    IconButton {
      id: deleteButton
      iconName: 'trash'
      innerOffset: 0
      visible: hoverHandler.hovered || root.selected

      onClicked: {
        root.deleteClicked();
      }
    }
  }
}
