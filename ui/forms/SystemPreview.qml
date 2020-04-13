import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12
import ChaosKit 1.0

SystemView {
  id: systemView

  property bool autoRefresh: true
  property int refreshInterval: 100
  property real zoom: 1.0
  property real translationX: 0
  property real translationY: 0

  colorMap: documentModel.documentProxy.colorMap
  colorMapRegistry: globalColorMapRegistry
  height: documentModel.documentProxy.height
  model: documentModel
  scale: 1.0 / Screen.devicePixelRatio
  transform: [
    Scale {
      origin.x: width * .5
      origin.y: height * .5
      xScale: zoom
      yScale: zoom
    },
    Translate {
      x: translationX
      y: translationY
    }
  ]
  width: documentModel.documentProxy.width

  Timer {
    interval: parent.refreshInterval
    running: parent.autoRefresh && parent.running
    repeat: true
    onTriggered: {
      systemView.update();
    }
  }

  // Panning

  DragHandler {
    id: dragHandler
    target: null

    property real initialTranslationX
    property real initialTranslationY
    onActiveChanged: {
      if (active) {
        initialTranslationX = systemView.translationX;
        initialTranslationY = systemView.translationY;
      }
    }
    onTranslationChanged: {
      systemView.translationX = initialTranslationX + translation.x;
      systemView.translationY = initialTranslationY + translation.y;
    }
  }

  NumberAnimation {
    id: panResetAnimation
    duration: 150
    easing.type: Easing.InOutQuad
    target: systemView
    properties: "translationX,translationY"
    to: 0
  }

  // Zooming

  readonly property var presetZoomLevels: [
    .002, .003, .004, .005, .007, .01, .015, .02, .03, .04, .05, .0625, .0833,
    .167, .25, .333, .5, .667, 1., 2., 3., 4., 5., 6., 7., 8., 12., 16., 32.,
  ]

  PinchHandler {
    id: zoomHandler
    target: null
    minimumRotation: 0
    maximumRotation: 0

    // We need initialZoom to play well with keyboard-based zooming.
    property real initialZoom

    onActiveChanged: {
      if (active) {
        // activeScale doesn't get reset to 1.0 every time a pinch happens.
        // Since it's readonly, we need to correct it ourselves.
        initialZoom = systemView.zoom / activeScale;
      }
    }

    onActiveScaleChanged: {
      systemView.zoom = initialZoom * activeScale;
    }
  }

  SmoothedAnimation {
    id: zoomAnimation
    target: systemView
    property: 'zoom'
    duration: 150
    velocity: -1
  }

  Shortcut {
    sequences: [StandardKey.ZoomIn, "Ctrl+="]
    onActivated: {
      const currentZoom = zoomAnimation.running ? zoomAnimation.to : zoom;
      const nextZoom = presetZoomLevels.find(level => level > currentZoom);
      if (nextZoom) {
        zoomAnimation.to = nextZoom;
        zoomAnimation.restart();
      }
    }
  }
  Shortcut {
    sequence: StandardKey.ZoomOut
    onActivated: {
      const currentZoom = zoomAnimation.running ? zoomAnimation.to : zoom;
      let nextZoom;
      for (let i = presetZoomLevels.length - 1; i >= 0; --i) {
        if (presetZoomLevels[i] < currentZoom) {
          nextZoom = presetZoomLevels[i];
          break;
        }
      }

      if (nextZoom) {
        zoomAnimation.to = nextZoom;
        zoomAnimation.restart();
      }
    }
  }
  // Zoom to fit
  Shortcut {
    sequence: "Ctrl+0"
    onActivated: {
      const scaleX =
          systemView.Window.width / (
              systemView.width * systemView.scale + Theme.windowPadding);
      const scaleY =
          systemView.Window.height / (
              systemView.height * systemView.scale + Theme.windowPadding);

      zoomAnimation.to = Math.min(1, scaleX, scaleY);
      zoomAnimation.restart();
      panResetAnimation.start();
    }
  }
  // Zoom to 100%
  Shortcut {
    sequence: "Ctrl+1"
    onActivated: {
      zoomAnimation.to = 1;
      zoomAnimation.restart();
      panResetAnimation.start();
    }
  }
}
