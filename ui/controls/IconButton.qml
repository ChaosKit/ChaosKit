import QtQuick 2.0
import QtQuick.Controls 2.12

RoundButton {
  property string iconName

  icon.source: `qrc:/icons/${iconName}.svg`
  icon.width: 16
  icon.height: 16
  bottomInset: 0
  flat: true
  leftInset: 0
  padding: 0
  rightInset: 0
  spacing: 0
  topInset: 0
}
