import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import ChaosKit 1.0

T.Slider {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitHandleWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitHandleHeight + topPadding + bottomPadding)

  padding: 0

  handle: Rectangle {
    x: control.leftPadding + (control.horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
    y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))
    implicitWidth: Theme.units(3)
    implicitHeight: Theme.units(3)
    radius: width / 2
    color: Theme.primaryColor
  }

  background: Rectangle {
    x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
    y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
    implicitWidth: control.horizontal ? 200 : 2
    implicitHeight: control.horizontal ? 2 : 200
    width: control.horizontal ? control.availableWidth : implicitWidth
    height: control.horizontal ? implicitHeight : control.availableHeight
    radius: 3
    color: Theme.alpha(Theme.primaryColor, 0.38)
    scale: control.horizontal && control.mirrored ? -1 : 1

    Rectangle {
      y: control.horizontal ? 0 : control.visualPosition * parent.height
      width: control.horizontal ? control.position * parent.width : 2
      height: control.horizontal ? 2 : control.position * parent.height

      radius: 1
      color: Theme.primaryColor
    }
  }
}
