import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T

T.SpinBox {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          contentItem.implicitWidth + 2 * padding +
                          up.implicitIndicatorWidth +
                          down.implicitIndicatorWidth)
  implicitHeight: Math.max(implicitContentHeight + topPadding + bottomPadding,
                           implicitBackgroundHeight,
                           up.implicitIndicatorHeight,
                           down.implicitIndicatorHeight)

  padding: 0
  leftPadding: padding + (control.mirrored ? (up.indicator ? up.indicator.width : 0) : (down.indicator ? down.indicator.width : 0))
  rightPadding: padding + (control.mirrored ? (down.indicator ? down.indicator.width : 0) : (up.indicator ? up.indicator.width : 0))

  validator: IntValidator {
    locale: control.locale.name
    bottom: Math.min(control.from, control.to)
    top: Math.max(control.from, control.to)
  }

  contentItem: TextInput {
    z: 2
    text: control.displayText

    font: control.font
    color: Theme.onSurfaceHigh
    selectionColor: Theme.primaryColor
    selectedTextColor: control.enabled ? Theme.onPrimaryHigh : Theme.onPrimaryDisabled
    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter

    readOnly: !control.editable
    validator: control.validator
    inputMethodHints: control.inputMethodHints
  }

  up.indicator: Item {
    x: control.mirrored ? 0 : parent.width - width
    height: parent.height
    implicitWidth: 28
    implicitHeight: 28

    Rectangle {
      anchors.fill: parent
      anchors.margins: 2
      color: Theme.controlColor(control.up.hovered, control.up.pressed)
      radius: 12
    }

    Rectangle {
      x: (parent.width - width) / 2
      y: (parent.height - height) / 2
      width: parent.width / 3
      height: 1
      color: enabled ? Theme.primaryColor : Theme.onSurfaceDisabled
    }
    Rectangle {
      x: (parent.width - width) / 2
      y: (parent.height - height) / 2
      width: 1
      height: parent.width / 3
      color: enabled ? Theme.primaryColor : Theme.onSurfaceDisabled
    }
  }

  down.indicator: Item {
    x: control.mirrored ? parent.width - width : 0
    height: parent.height
    implicitWidth: 28
    implicitHeight: 28

    Rectangle {
      anchors.fill: parent
      anchors.margins: 2
      color: Theme.controlColor(control.down.hovered, control.down.pressed)
      radius: 12
    }

    Rectangle {
      x: (parent.width - width) / 2
      y: (parent.height - height) / 2
      width: parent.width / 3
      height: 1
      color: enabled ? Theme.primaryColor : Theme.onSurfaceDisabled
    }
  }

  background: Rectangle {
    implicitWidth: 140
    color: Theme.surfaceOverlay

    Rectangle {
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      color: control.activeFocus ? Theme.primaryColor : Theme.onSurfaceMedium
      height: 1
      visible: control.enabled
    }
  }
}
