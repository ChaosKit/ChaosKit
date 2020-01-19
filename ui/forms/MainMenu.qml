import Qt.labs.platform 1.1
import QtQuick 2.12

MenuBar {
  id: menuBar
  signal exportImage()

  Menu {
    title: "File"
    MenuItem {
      text: "Export image…"

      onTriggered: {
        menuBar.exportImage()
      }
    }
    MenuItem {
      shortcut: StandardKey.Quit
      role: MenuItem.QuitRole
      text: "&Quit"

      onTriggered: {
        Qt.quit()
      }
    }
  }
}
