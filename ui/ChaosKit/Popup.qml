import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import ChaosKit 1.0

T.Popup {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          contentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           contentHeight + topPadding + bottomPadding)

  padding: Theme.units(3)

  background: Rectangle {
    radius: Theme.borderRadius
    color: Theme.blend(Theme.surfaceColor, Theme.white, 0.16)
  }

  enter: Transition {
    NumberAnimation {
      property: "opacity"
      from: 0.0
      to: 1.0
      easing.type: Easing.OutQuad
      duration: 150
    }
    NumberAnimation {
      property: "scale"
      from: 0.9
      to: 1.0
      easing.type: Easing.OutQuad
      duration: 150
    }
  }
  exit: Transition {
    NumberAnimation {
      property: "opacity"
      from: 1.0
      to: 0.0
      easing.type: Easing.InQuad
      duration: 75
    }
  }

  T.Overlay.modal: Rectangle {
    color: Qt.rgba(0, 0, 0, 0.32)
  }

  T.Overlay.modeless: Rectangle {
    color: Qt.rgba(0, 0, 0, 0.32)
  }
}
