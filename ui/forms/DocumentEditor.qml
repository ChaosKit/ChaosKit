import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

RowLayout {
  spacing: 0

  readonly property bool opened: tabs.currentIndex >= 0

  Separator {
    direction: Separator.Direction.Vertical
    visible: tabs.currentIndex >= 0
  }
  Rectangle {
    Layout.fillHeight: true
    Layout.preferredWidth: 250
    color: "#121212"
    visible: tabs.currentIndex >= 0
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
