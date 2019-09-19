import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.12

Pane {
  id: root

  property string type
  signal clicked()

  contentWidth: column.implicitWidth
  contentHeight: column.implicitHeight
  padding: 0

  ColumnLayout {
    id: column
    spacing: 0

    Image {
      Layout.preferredWidth: 100
      Layout.preferredHeight: 100

      source: "image://formula/" + type
    }

    Label {
      Layout.fillWidth: true
      bottomPadding: 4
      horizontalAlignment: Text.AlignHCenter
      text: modelData
    }
  }

  Rectangle {
    id: overlay
    anchors.fill: parent
    color: "#fff"
    opacity: 0

    states: State {
      name: "hovered"
      PropertyChanges {
        target: overlay
        opacity: 0.04
      }
    }

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      hoverEnabled: true

      onEntered: overlay.state = "hovered"
      onExited: overlay.state = ""
      onClicked: root.clicked()
    }
  }
}
