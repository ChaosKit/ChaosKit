import QtQuick
import QtQuick.Controls

RoundButton {
  property string iconName
  property int size: Theme.smallIconSize
  property int innerOffset: Theme.units(1)

  icon.source: Theme.iconUrl(iconName)
  icon.width: size
  icon.height: size
  bottomInset: 0
  flat: true
  implicitHeight: size + innerOffset * 2
  leftInset: 0
  padding: 0
  rightInset: 0
  spacing: 0
  topInset: 0
  implicitWidth: size + innerOffset * 2
}
