import QtQuick
import QtQuick.Controls
import ChaosKit

TabButton {
  id: button

  font.pointSize: Theme.buttonFontSize
  font.letterSpacing: Theme.letterSpacing(Theme.buttonFontSize)
  font.weight: Font.Medium
  horizontalPadding: Theme.units(3)

  Component.onCompleted: {
    background.implicitHeight = Theme.units(14);
  }
}
