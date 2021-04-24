import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import ChaosKit 1.0

T.CheckBox {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding,
                           implicitIndicatorHeight + topPadding + bottomPadding)

  padding: 0
  spacing: Theme.padding

  // keep in sync with CheckDelegate.qml (shared CheckIndicator.qml was removed for performance reasons)
  indicator: Rectangle {
    implicitWidth: Theme.units(3)
    implicitHeight: Theme.units(3)

    x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
    y: control.topPadding + (control.availableHeight - height) / 2

    color: control.checkState === Qt.Unchecked ? 'transparent' : Theme.primaryColor
    border.width: control.checkState === Qt.Unchecked ? 2 : 0
    border.color: Theme.onSurfaceMedium
    radius: 1

    Icon {
      x: (parent.width - width) / 2
      y: (parent.height - height) / 2
      tint: Theme.onPrimaryHigh
      name: 'checkbox'
      size: parent.width
      visible: control.checkState === Qt.Checked
    }

    Rectangle {
      x: (parent.width - width) / 2
      y: (parent.height - height) / 2
      width: parent.width - Theme.unit
      height: 2
      color: Theme.onPrimaryHigh
      visible: control.checkState === Qt.PartiallyChecked
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
