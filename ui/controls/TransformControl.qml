import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
  id: root
  property color color

  Rectangle {
    anchors.fill: parent
    border.width: 1
    border.color: parent.color
    color: "transparent"

    Rectangle {
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.topMargin: -3
      anchors.leftMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeFDiagCursor
      }
    }

    Rectangle {
      anchors.top: parent.top
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.topMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeVerCursor
      }
    }

    Rectangle {
      anchors.top: parent.top
      anchors.right: parent.right
      anchors.topMargin: -3
      anchors.rightMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeBDiagCursor
      }
    }

    Rectangle {
      anchors.verticalCenter: parent.verticalCenter
      anchors.left: parent.left
      anchors.leftMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeHorCursor
      }
    }

    Rectangle {
      anchors.verticalCenter: parent.verticalCenter
      anchors.right: parent.right
      anchors.rightMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeHorCursor
      }
    }

    Rectangle {
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.bottomMargin: -3
      anchors.leftMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeBDiagCursor
      }
    }

    Rectangle {
      anchors.bottom: parent.bottom
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottomMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeVerCursor
      }
    }

    Rectangle {
      anchors.bottom: parent.bottom
      anchors.right: parent.right
      anchors.bottomMargin: -3
      anchors.rightMargin: -3
      color: root.color
      width: 8
      height: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeFDiagCursor
      }
    }
  }
}
