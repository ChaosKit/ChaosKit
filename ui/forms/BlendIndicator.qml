import QtQuick 2.12
import QtQuick.Controls 2.12
import ChaosKit 1.0

Rectangle {
  id: item

  property real weight
  signal enabled()
  signal disabled()
  signal isolated()

  border.color: Theme.onSurfaceMedium
  border.width: 1
  color:
      Theme.blend(
          Theme.surfaceColor,
          Theme.onSurface,
          (hoverHandler.hovered || dragHandler.active)
              ? Theme.hoverRatio : 0
      )
  height: Theme.blendIndicatorHeight
  radius: Theme.borderRadius
  width: Theme.blendIndicatorWidth

  states: [
    State {
      name: 'disabled'

      PropertyChanges {
        target: item
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
        target: item
        border.color: Theme.primaryColor
      }
      PropertyChanges {
        target: label
        color: Theme.primaryColor
        text: '∞'
      }
    }
  ]

  HoverHandler {
    id: hoverHandler
    // TODO: support custom mouse cursors
  }

  TapHandler {
    acceptedModifiers: Qt.ControlModifier
    onTapped: {
      if (item.state === 'disabled') {
        item.enabled();
      } else {
        item.disabled();
      }
    }
  }

  TapHandler {
    acceptedModifiers: Qt.AltModifier
    onTapped: {
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
      item.weight = Math.max(startWeight + translation.x * .01, 0);
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
    text: Math.round(weight * 100) / 100
    verticalAlignment: Text.AlignVCenter
  }
}
