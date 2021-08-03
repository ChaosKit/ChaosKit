import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import ChaosKit

T.ComboBox {
  id: control

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding,
                           implicitIndicatorHeight + topPadding + bottomPadding)

  leftPadding: padding + (!control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
  rightPadding: padding + (control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)

  delegate: ItemDelegate {
    width: ListView.view.width
    text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
    highlighted: control.highlightedIndex === index || control.currentIndex === index
    hoverEnabled: control.hoverEnabled
  }

  indicator: Icon {
    name: control.popup.visible ? "arrow-up" : "arrow-down"
    tint: control.activeFocus ? Theme.primaryColor : Theme.onSurfaceMedium
    size: Theme.smallIconSize
    x: control.mirrored ? control.padding + Theme.units(2) : control.width - width - control.padding - Theme.units(2)
    y: control.topPadding + (control.availableHeight - height) / 2
  }

  contentItem: T.TextField {
    leftPadding: !control.mirrored ? Theme.units(2) : control.editable && activeFocus ? 3 : 1
    rightPadding: control.mirrored ? Theme.units(2) : control.editable && activeFocus ? 3 : 1
    topPadding: Theme.units(1) - control.padding
    bottomPadding: Theme.units(1) - control.padding

    text: control.editable ? control.editText : control.displayText

    enabled: control.editable
    autoScroll: control.editable
    readOnly: control.down
    inputMethodHints: control.inputMethodHints
    validator: control.validator
    selectByMouse: control.selectTextByMouse

    font: control.font
    color: Theme.onSurfaceHigh
    selectionColor: Theme.primaryColor
    selectedTextColor: Theme.onPrimaryHigh
    verticalAlignment: Text.AlignVCenter
  }

  background: Rectangle {
    implicitWidth: 200
    implicitHeight: 28

    color: Theme.surfaceOverlay
    visible: !control.flat || control.down

    Rectangle {
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      color: control.activeFocus ? Theme.primaryColor : Theme.onSurfaceMedium
      height: 1
      visible: control.enabled
    }
  }

  popup: Popup {
    y: control.height
    width: control.width
    height: Math.min(contentItem.implicitHeight, control.Window.height - topMargin - bottomMargin)
    topMargin: Theme.units(2)
    bottomMargin: Theme.units(2)
    padding: 0

    contentItem: ListView {
      clip: true
      implicitHeight: contentHeight
      model: control.delegateModel
      currentIndex: control.highlightedIndex
      highlightMoveDuration: 0

      Rectangle {
        z: 10
        width: parent.width
        height: parent.height
        color: "transparent"
      }

      T.ScrollIndicator.vertical: ScrollIndicator { }
    }
  }
}
