import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T

T.ToolTip {
  id: control

  x: parent ? (parent.width - implicitWidth) / 2 : 0
  y: -implicitHeight - Theme.units(2)

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          contentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           contentHeight + topPadding + bottomPadding)

  margins: 0
  horizontalPadding: Theme.units(2)
  verticalPadding: (Theme.units(5) - Theme.captionFontSize) / 2

  closePolicy:
    T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent |
    T.Popup.CloseOnReleaseOutsideParent

  contentItem: Text {
    text: control.text
    font.pointSize: Theme.captionFontSize
    font.letterSpacing: Theme.letterSpacing(Theme.captionFontSize)
    color: Theme.surfaceColor
  }

  background: Rectangle {
    // color: Theme.onSurfaceMedium
    color: Theme.blend(Theme.surfaceColor, Theme.onSurface, 0.6)
    radius: Theme.borderRadius
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
}
