import QtQuick.Controls.impl 2.15

IconLabel {
  property string name
  property color tint: Theme.onSurfaceHigh
  property int size: Theme.smallIconSize

  icon.color: tint
  icon.source: Theme.iconUrl(name)
  icon.width: size
  icon.height: size
}
