import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

GridLayout {
  columns: 2
  columnSpacing: Theme.padding
  rowSpacing: Theme.smallPadding

  Heading {
    Layout.columnSpan: 2

    padding: Theme.padding
    text: "Color Map"
  }

  Label {
    leftPadding: Theme.padding
    text: "Preset"
  }
  ComboBox {
    id: colorMapPicker

    Layout.fillWidth: true
    Layout.rightMargin: Theme.padding

    currentIndex: projectModel.colorMap.index
    model: projectModel.colorMap.registry.names
    delegate: ItemDelegate {
      id: itemDelegate
      contentItem: Label {
        font.weight: Font.DemiBold
        text: modelData
        style: Text.Outline
        styleColor: Theme.onPrimaryHigh
      }
      background: Image {
        width: colorMapPicker.implicitWidth
        height: colorMapPicker.implicitHeight + Theme.units(1)
        source: 'image://colormap/' + modelData

        Rectangle {
          anchors.fill: parent
          color: Theme.onSurface
          opacity: itemDelegate.hovered ? 0.12 : 0.0
        }
      }
    }

    onActivated: {
      projectModel.colorMap.name = currentText;
    }
  }

  Image {
    Layout.columnSpan: 2
    Layout.fillWidth: true
    Layout.preferredHeight: Theme.units(1)
    Layout.leftMargin: Theme.padding
    Layout.rightMargin: Theme.padding
    source: 'image://colormap/' + projectModel.colorMap.name
  }
}
