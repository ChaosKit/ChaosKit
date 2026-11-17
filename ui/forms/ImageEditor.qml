import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

ColumnLayout {
  id: root

  property QtObject systemView: null

  spacing: Theme.padding

  ImageSizeEditor {}
  ToneMappingEditor {}
  ColorMapEditor {}
  AccumulationEditor {
    systemView: root.systemView

    Layout.fillWidth: true
  }
}
