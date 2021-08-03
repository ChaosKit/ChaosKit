import QtQuick
import QtQuick.Templates as T
import ChaosKit

T.Label {
  color: enabled ? Theme.onSurfaceHigh : Theme.onSurfaceDisabled
  font.pointSize: Theme.body1FontSize
}
