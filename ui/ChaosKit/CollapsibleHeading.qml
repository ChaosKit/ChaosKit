import QtQuick

Rectangle {
  id: rectangle

  color: Theme.alpha(
      Theme.onSurface,
      mouseArea.containsPress ? 0.1 :
      mouseArea.containsMouse ? 0.04 :
      0.0)
  implicitHeight: row.implicitHeight
  implicitWidth: row.implicitWidth

  property bool opened: false
  property alias text: heading.text

  Row {
    id: row
    leftPadding: Theme.padding
    padding: Theme.smallPadding
    spacing: Theme.smallPadding

    Heading {
      anchors.verticalCenter: parent.verticalCenter
      id: heading
    }

    IconButton {
      icon.color: Theme.onSurfaceMedium
      iconName: opened ? 'arrow-up' : 'arrow-down'

      Component.onCompleted: {
        background.color = 'transparent';
      }
    }
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true

    onClicked: {
      opened = !opened;
    }
  }
}
