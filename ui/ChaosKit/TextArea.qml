import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T

T.TextArea {
  id: control

  property bool outlined: false

  implicitWidth: Math.max(contentWidth + leftPadding + rightPadding,
                          implicitBackgroundWidth + leftInset + rightInset,
                          placeholder.implicitWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                           implicitBackgroundHeight + topInset + bottomInset,
                           placeholder.implicitHeight + topPadding + bottomPadding)

  leftPadding: Theme.units(3)
  rightPadding: Theme.units(3)
  padding: Theme.units(2)

  color: Theme.onSurfaceHigh
  placeholderTextColor: Theme.onSurfaceMedium
  selectionColor: Theme.primaryColor
  selectedTextColor: Theme.onPrimaryHigh

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
    implicitHeight: 40
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
