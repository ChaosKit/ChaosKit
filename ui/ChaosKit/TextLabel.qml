import QtQuick 2.15
import QtQuick.Controls 2.15
import ChaosKit 1.0

Label {
  property real fontSize: Theme.body1FontSize

  font.pointSize: fontSize
  font.letterSpacing: Theme.letterSpacing(fontSize)
}
