import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T

T.TextField {
  id: control

  property bool outlined: false

  implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                 || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           contentHeight + topPadding + bottomPadding,
                           placeholder.implicitHeight + topPadding + bottomPadding)

  leftPadding: Theme.units(2)
  rightPadding: Theme.units(2)
  padding: Theme.units(1)

  color: Theme.onSurfaceHigh
  placeholderTextColor: Theme.onSurfaceMedium
  selectionColor: Theme.primaryColor
  selectedTextColor:
      control.enabled ? Theme.onPrimaryHigh : Theme.onPrimaryDisabled
  verticalAlignment: TextInput.AlignVCenter

  PlaceholderText {
    id: placeholder
    x: control.leftPadding
    y: control.topPadding
    width: control.width - (control.leftPadding + control.rightPadding)
    height: control.height - (control.topPadding + control.bottomPadding)

    text: control.placeholderText
    font: control.font
    color: control.placeholderTextColor
    verticalAlignment: control.verticalAlignment
    visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
    elide: Text.ElideRight
    renderType: control.renderType
  }

  background: Rectangle {
    anchors.fill: parent
    implicitWidth: 200
    implicitHeight: 28
    color: control.outlined ? 'transparent' : Theme.surfaceOverlay
    border.color: control.activeFocus ? Theme.primaryColor : Theme.borderColor
    border.width: control.outlined ? 1 : 0

    Rectangle {
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      color: control.activeFocus ? Theme.primaryColor : Theme.onSurfaceMedium
      height: 1
      visible: control.enabled && !control.outlined
    }
  }
}
