import Qt.labs.platform
import QtQuick

MenuBar {
  id: menuBar
  signal newTriggered()
  signal open()
  signal save()
  signal saveAs()
  signal exportImage()

  Menu {
    title: "File"

    MenuItem {
      shortcut: StandardKey.New
      text: "New"

      onTriggered: {
        menuBar.newTriggered();
      }
    }
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
      // shortcut: StandardKey.SaveAs
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
      // shortcut: StandardKey.Quit
      role: MenuItem.QuitRole
      text: "&Quit"

      onTriggered: {
        Qt.quit()
      }
    }
  }
}
