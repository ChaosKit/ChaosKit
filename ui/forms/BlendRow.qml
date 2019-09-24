import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.11
import "../controls"

Item {
  id: root

  property var modelIndex
  property bool toggleVisible
  property alias drag: mouseArea.drag
  property bool isOpen: false

  signal dragEnded()

  implicitHeight: contents.implicitHeight

  Rectangle {
    id: background
    anchors.fill: parent
    color: Material.foreground
    opacity: selectionModel.currentIndex == parentIndex ? 0.25 :
        (mouseArea.containsMouse ? 0.1 : 0.0)
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true

    onClicked: {
      selectionModel.setCurrentIndex(
          modelIndex, ItemSelectionModel.ClearAndSelect);
    }
    onReleased: {
      if (mouseArea.drag.active) {
        root.dragEnded();
      }
    }
  }

  RowLayout {
    id: contents
    anchors.left: parent.left
    anchors.right: parent.right

    IconButton {
      iconName: model.enabled ? "eye" : "eye-slash"
      onClicked: model.enabled = !model.enabled
    }

    IconButton {
      id: openToggle
      iconName: isOpen ? "caret-down" : "caret-right"
      visible: toggleVisible
      onClicked: {
        isOpen = !isOpen;
      }
    }

    Label {
      Layout.fillWidth: true
      Layout.fillHeight: true

      text: display
      verticalAlignment: Text.AlignVCenter
      elide: Text.ElideRight
    }
  }

  // Close formulas if they should not be visible.
  Connections {
    target: documentModel
    onRowsRemoved: {
      if (parent === modelIndex && documentModel.rowCount(parent) <= 1) {
        row.isOpen = false;
      }
    }
  }
}
