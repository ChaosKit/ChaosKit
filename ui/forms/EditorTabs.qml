import QtQml.Models
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

Rectangle {
  id: root

  property int currentIndex: -1

  color: Theme.backgroundColor
  implicitHeight: layout.implicitHeight
  implicitWidth: layout.implicitWidth

  ColumnLayout {
    id: layout
    anchors.fill: parent
    spacing: 0

    ListModel {
      id: tabs
      ListElement {
        icon: 'solar-system'
        text: 'System'
      }
      ListElement {
        icon: 'image'
        text: 'Image'
      }
    }

    Repeater {
      id: repeater
      model: tabs
      delegate: EditorTabButton {
        Layout.fillWidth: true
        checked: root.currentIndex === index
        display: AbstractButton.TextUnderIcon
        icon.source: model.icon ? `qrc:/icons/${model.icon}.svg` : ''
        text: model.text

        onClicked: {
          root.currentIndex = (root.currentIndex === index) ? -1 : index;
          animateIndicator();
        }
      }
    }

    MouseArea {
      Layout.fillWidth: true
      Layout.fillHeight: true

      onClicked: {
        root.currentIndex = -1;
      }
    }
  }

  Rectangle {
    id: indicator
    color: Theme.primaryColor
    opacity: root.currentIndex >= 0 ? 1 : 0
    width: 2
    x: 0
    y: 0

    Component.onCompleted: {
      const dimensionIndex = Math.max(root.currentIndex, 0);
      const item = repeater.itemAt(dimensionIndex);
      if (root.currentIndex >= 0) {
        indicator.y = item.y;
      }
      indicator.height = item.implicitHeight;
    }

    Behavior on y {
      NumberAnimation {
        duration: 150
        easing.type: Easing.Bezier
        easing.bezierCurve: Theme.standardCurve
      }
    }
    Behavior on height {
      NumberAnimation {
        duration: 150
        easing.type: Easing.Bezier
        easing.bezierCurve: Theme.standardCurve
      }
    }
    Behavior on opacity {
      NumberAnimation {
        duration: 75
        easing.type: Easing.Bezier
        easing.bezierCurve: Theme.standardCurve
      }
    }
  }

  function animateIndicator() {
    if (root.currentIndex < 0) return;

    const item = repeater.itemAt(root.currentIndex);
    indicator.y = item.y;
    indicator.height = item.implicitHeight;
  }
}
