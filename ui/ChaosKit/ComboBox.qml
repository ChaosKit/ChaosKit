import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import ChaosKit 1.0

T.ComboBox {
  id: control

  property bool outlined: false

  implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                          implicitContentWidth + leftPadding + rightPadding)
  implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                           implicitContentHeight + topPadding + bottomPadding,
                           implicitIndicatorHeight + topPadding + bottomPadding)

  leftPadding: padding + (!control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
  rightPadding: padding + (control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)

  delegate: ItemDelegate {
    width: parent.width
    text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
    highlighted: control.highlightedIndex === index || control.currentIndex === index
    hoverEnabled: control.hoverEnabled
  }

  indicator: IconButton {
    iconName: control.popup.visible ? "arrow-up" : "arrow-down"
    icon.color: control.activeFocus ? Theme.primaryColor : Theme.onSurfaceMedium
    size: Theme.smallIconSize
    x: control.mirrored ? control.padding : control.width - width - control.padding
    y: control.topPadding + (control.availableHeight - height) / 2

    background: null
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

    font: control.font
    color: Theme.onSurfaceHigh
    selectionColor: Theme.primaryColor
    selectedTextColor: Theme.onPrimaryHigh
    verticalAlignment: Text.AlignVCenter
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
