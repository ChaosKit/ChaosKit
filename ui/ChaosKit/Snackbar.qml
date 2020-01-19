import QtQuick 2.12
import QtQuick.Controls 2.12

Popup {
  id: snackbar
  property bool autoClose: true
  property alias text: label.text

  margins: Theme.units(2)
  parent: Overlay.overlay
  x: Math.round((parent.width - width) / 2)
  y: parent.height - height

  Label {
    id: label
  }

  Timer {
    interval: 4000
    running: autoClose && opened
    onTriggered: {
      snackbar.close();
    }
  }
}
