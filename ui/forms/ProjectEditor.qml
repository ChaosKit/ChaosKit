import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

RowLayout {
  spacing: 0

  readonly property bool opened: tabs.currentIndex >= 0
  property alias currentIndex: tabs.currentIndex

  Separator {
    direction: Separator.Direction.Vertical
    visible: tabs.currentIndex >= 0
  }
  Rectangle {
    Layout.fillHeight: true
    Layout.preferredWidth: 250
    color: Theme.backgroundColor
    visible: tabs.currentIndex >= 0

    StackLayout {
      anchors.fill: parent
      currentIndex: tabs.currentIndex

      SystemEditor {}
      ImageEditor {}
    }
  }
  Separator {
    direction: Separator.Direction.Vertical
  }
  EditorTabs {
    id: tabs
    Layout.alignment: Qt.AlignTop
    Layout.fillHeight: true
  }
}
