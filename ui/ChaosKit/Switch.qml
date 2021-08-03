import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls
import QtQuick.Controls.impl
import ChaosKit

T.Switch {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding,
                           implicitIndicatorHeight + topPadding + bottomPadding)

  padding: 0
  spacing: Theme.padding

  indicator: PaddedRectangle {
    implicitWidth: Theme.units(7)
    implicitHeight: Theme.units(4)

    x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
    y: control.topPadding + (control.availableHeight - height) / 2

    radius: Theme.units(3) / 2
    leftPadding: 0
    rightPadding: 0
    padding: (height - Theme.units(3)) / 2
    color: control.checked ?
        Theme.alpha(Theme.primaryColor, 0.32) :
        Theme.alpha(Theme.onSurface, 0.15)

    Rectangle {
      x: Math.max(0, Math.min(parent.width - width, control.visualPosition * parent.width - (width / 2)))
      y: (parent.height - height) / 2
      width: Theme.units(4)
      height: Theme.units(4)
      radius: Theme.units(2)
      color: control.checked ?
          Theme.primaryColor
          : Theme.blend(Theme.surfaceColor, Theme.onSurface, 0.63)

      Behavior on x {
        enabled: !control.down
        NumberAnimation {
          duration: 100
          easing.bezierCurve: Theme.standardCurve
        }
      }
    }
  }

  contentItem: CheckLabel {
    leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
    rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0

    text: control.text
    font: control.font
    color: Theme.onSurfaceHigh
  }
}
