import QtQuick 2.12
import ChaosKit 1.0

Rectangle {
  border.color: Theme.onSurfaceMedium
  border.width: 1
  color: Theme.surfaceColor
  height: Theme.blendIndicatorHeight / Math.SQRT2 + 1
  rotation: 45
  transform: [ Translate {x: 0; y: 1} ]  // pixel-pushing
  width: Theme.blendIndicatorHeight / Math.SQRT2 + 1
}
