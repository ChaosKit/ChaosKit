import QtQuick
import QtQuick.Controls
import ChaosKit

Label {
  property real fontSize: Theme.body1FontSize

  font.pointSize: fontSize
  font.letterSpacing: Theme.letterSpacing(fontSize)
}
