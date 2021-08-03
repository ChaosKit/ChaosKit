import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

ScrollView {
  id: root

  required property var system

  clip: true
  contentWidth: availableWidth

  ColumnLayout {
    spacing: 0
    width: parent.width

    Heading {
      Layout.leftMargin: Theme.padding
      Layout.topMargin: Theme.padding
      text: 'System Settings'
    }

    LifetimeEditor {
      Layout.fillWidth: true
      Layout.leftMargin: Theme.padding
      Layout.rightMargin: Theme.padding
      Layout.topMargin: Theme.largePadding
      system: root.system
    }
  }
}
