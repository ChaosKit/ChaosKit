import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl 2.15

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
