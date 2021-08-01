import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl

IconLabel {
  property string name
  property color tint: Theme.onSurfaceHigh
  property int size: Theme.smallIconSize
  property string tooltip: ''

  icon.color: tint
  icon.source: Theme.iconUrl(name)
  icon.width: size
  icon.height: size

  ToolTip.delay: Theme.toolTipDelay
  ToolTip.text: tooltip
  ToolTip.visible: hoverHandler.hovered

  HoverHandler {
    id: hoverHandler
    enabled: !!tooltip
  }
}
