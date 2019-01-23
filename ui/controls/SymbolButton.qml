import QtQuick 2.0
import "../resources"

Button {
  id: button

  property alias symbol: button.text

  font.family: Fonts.icons
  font.bold: true  // TODO: support regular icons
}
