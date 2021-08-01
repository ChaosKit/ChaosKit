import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import ChaosKit

T.RadioButton {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding,
                           implicitIndicatorHeight + topPadding + bottomPadding)

  padding: 0
  spacing: Theme.smallPadding

  indicator: Rectangle {
    implicitWidth: Theme.units(4)
    implicitHeight: Theme.units(4)

    x: text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
    y: control.topPadding + (control.availableHeight - height) / 2

    radius: width / 2
    color: 'transparent'
    border.width: 2
    border.color: control.checked ? Theme.primaryColor : Theme.onSurfaceMedium

    Rectangle {
      x: (parent.width - width) / 2
      y: (parent.height - height) / 2
      width: Theme.units(2)
      height: Theme.units(2)
      radius: width / 2
      color: Theme.primaryColor
      visible: control.checked
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
