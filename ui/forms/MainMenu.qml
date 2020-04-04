import Qt.labs.platform 1.1
import QtQuick 2.12

MenuBar {
  id: menuBar
  signal open()
  signal save()
  signal saveAs()
  signal exportImage()

  Menu {
    title: "File"

    MenuItem {
      shortcut: StandardKey.Open
      text: "Open"

      onTriggered: {
        menuBar.open();
      }
    }
    MenuItem {
      shortcut: StandardKey.Save
      text: "Save"

      onTriggered: {
        menuBar.save();
      }
    }
    MenuItem {
      shortcut: StandardKey.SaveAs
      text: "Save as…"

      onTriggered: {
        menuBar.saveAs();
      }
    }

    MenuSeparator {}

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
