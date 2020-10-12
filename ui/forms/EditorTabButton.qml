import QtQuick 2.12
import QtQuick.Controls 2.12
import ChaosKit 1.0

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
