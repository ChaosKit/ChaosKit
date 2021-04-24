import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import ChaosKit 1.0

Item {
  ScrollView {
    anchors.bottom: separator.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top

    StructureEditor {
      width: parent.width
    }
  }

  Separator {
    id: separator
    anchors.left: parent.left
    anchors.right: parent.right
    draggable: true
    minPosition: 100
    y: 200
  }

  Item {
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: separator.bottom
  }
}

