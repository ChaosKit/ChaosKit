import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

GridLayout {
  readonly property var document: documentModel.documentProxy

  property string currentColorMap
  Binding on currentColorMap {
    value: document.colorMap
  }

  columns: 2
  columnSpacing: Theme.padding
  rowSpacing: Theme.smallPadding

  onCurrentColorMapChanged: {
    colorMapPicker.currentIndex =
        colorMapPicker.model.indexOf(document.colorMap);
  }

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

    model: globalColorMapRegistry.names
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
        source: `image://colormap/${modelData}`

        Rectangle {
          anchors.fill: parent
          color: Theme.onSurface
          opacity: itemDelegate.hovered ? 0.12 : 0.0
        }
      }
    }

    onActivated: {
      document.colorMap = currentText;
    }
  }

  Image {
    Layout.columnSpan: 2
    Layout.fillWidth: true
    Layout.preferredHeight: Theme.units(1)
    Layout.leftMargin: Theme.padding
    Layout.rightMargin: Theme.padding
    source: `image://colormap/${document.colorMap}`
  }
}
