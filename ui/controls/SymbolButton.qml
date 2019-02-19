import QtQuick 2.0
import QtQuick.Controls 2.12
import "../resources"

RoundButton {
  id: button

  property alias symbol: button.text

  bottomInset: 0
  flat: true
  font.family: Fonts.icons
  font.bold: true  // TODO: support regular icons
  leftInset: 0
  padding: 0
  rightInset: 0
  spacing: 0
  topInset: 0
}
