import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import ChaosKit 1.0

T.Label {
  color: enabled ? Theme.onSurfaceHigh : Theme.onSurfaceDisabled
  font.pointSize: Theme.body1FontSize
}
