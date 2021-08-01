import QtQuick
import QtQuick.Controls
import ChaosKit

Item {
  id: item

  property real weight
  signal enabled()
  signal disabled()
  signal isolated()
  signal weightEdited(real weight)

  implicitHeight: textField.visible ? textField.height : rect.height
  width: Theme.weightIndicatorWidth

  states: [
    State {
      name: 'disabled'

      PropertyChanges {
        target: rect
        border.color: Theme.onSurfaceDisabled
      }
      PropertyChanges {
        target: label
        color: Theme.onSurfaceDisabled
        text: 'X'
      }
    },
    State {
      name: 'isolated'

      PropertyChanges {
        target: rect
        border.color: Theme.primaryColor
      }
      PropertyChanges {
        target: label
        color: Theme.primaryColor
        text: '∞'
      }
    },
    State {
      name: 'editing'

      PropertyChanges {
        target: rect
        visible: false
      }
      PropertyChanges {
        target: textField
        visible: true
      }
    }
  ]

  Rectangle {
    id: rect
    border.color: Theme.onSurfaceMedium
    border.width: 1
    color:
        Theme.blend(
            Theme.surfaceColor,
            Theme.onSurface,
            (hoverHandler.hovered || dragHandler.active)
                ? Theme.hoverRatio : 0
        )
    height: Theme.weightIndicatorHeight
    radius: Theme.borderRadius
    width: parent.width

    HoverHandler {
      id: hoverHandler
      // TODO: support custom mouse cursors
    }

    TapHandler {
      onDoubleTapped: {
        item.state = 'editing';
        textField.focus = true;
      }
    }

    TapHandler {
      acceptedModifiers: Qt.ControlModifier
      onSingleTapped: {
        if (item.state === 'disabled') {
          item.enabled();
        } else {
          item.disabled();
        }
      }
    }

    TapHandler {
      acceptedModifiers: Qt.AltModifier
      onSingleTapped: {
        if (item.state === 'isolated') {
          item.enabled();
        } else {
          item.isolated();
        }
      }
    }

    DragHandler {
      id: dragHandler
      target: null

      property real startWeight: item.weight

      onActiveChanged: {
        if (active) {
          startWeight = item.weight;
        }
      }
      onTranslationChanged: {
        const weight = Math.max(startWeight + translation.x * .01, 0);
        item.weight = weight;
        item.weightEdited(weight);
      }
    }

    Label {
      id: label
      anchors.fill: parent
      anchors.margins: 1
      color: Theme.onSurfaceHigh
      font.pointSize: 8
      font.letterSpacing: Theme.letterSpacing(8)
      font.weight: Font.DemiBold
      horizontalAlignment: Text.AlignHCenter
      text: Math.round(item.weight * 100) / 100
      verticalAlignment: Text.AlignVCenter
    }
  }

  // Weight editor as text

  TextField {
    id: textField
    anchors.centerIn: parent
    font.pointSize: 8
    font.letterSpacing: Theme.letterSpacing(8)
    font.weight: Font.DemiBold
    height: Theme.weightIndicatorHeight + Theme.smallPadding * 2
    leftPadding: 2
    padding: 0
    rightPadding: 2
    text: Math.round(item.weight * 100) / 100
    validator: DoubleValidator {
      bottom: 0
      locale: 'en_US'
      notation: DoubleValidator.StandardNotation
    }
    visible: false
    width: parent.width

    Keys.onEscapePressed: {
      item.state = '';
      event.accepted = true;
    }

    onEditingFinished: {
      const weight = Number(text);
      item.weight = weight;
      item.weightEdited(weight);

      // Close the editor
      item.state = '';
    }
  }
}
