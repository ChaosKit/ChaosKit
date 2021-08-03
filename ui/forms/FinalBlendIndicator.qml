import QtQuick
import ChaosKit

Rectangle {
  border.color: Theme.onSurfaceMedium
  border.width: 1
  color: Theme.surfaceColor
  height: Theme.weightIndicatorHeight / Math.SQRT2 + 1
  rotation: 45
  transform: [ Translate {x: 0; y: 1} ]  // pixel-pushing
  width: Theme.weightIndicatorHeight / Math.SQRT2 + 1
}
