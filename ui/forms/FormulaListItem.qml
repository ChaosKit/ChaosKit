import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Controls.Material 2.12
import "../controls"
import "../resources"

MouseArea {
  id: itemRoot
  implicitHeight: contents.implicitHeight
  hoverEnabled: true

  property bool itemEnabled: true  // TODO: use the model
  property var rootModel: null
  property var blendModel: null
  property var selectionModel: null
  readonly property var selectionIndex: blendModel.modelIndexForSelection(index)

  onClicked: {
    selectionModel.setCurrentIndex(
        selectionIndex, ItemSelectionModel.ClearAndSelect);
  }

  Rectangle {
    id: background
    anchors.fill: parent
    color: Material.foreground
    opacity: selectionModel.currentIndex == selectionIndex ? 0.25 :
        (parent.containsMouse ? 0.1 : 0.0)
  }

  RowLayout {
    id: contents
    anchors.fill: parent
    spacing: 0

    SymbolButton {
      symbol: itemRoot.itemEnabled ? Icons.faEye : Icons.faEyeSlash
      onClicked: itemRoot.itemEnabled = !itemRoot.itemEnabled
    }

    Label {
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.leftMargin: 12

      text: display
      verticalAlignment: Text.AlignVCenter
      elide: Text.ElideRight
    }

    Slider {
      Layout.preferredWidth: 100
      Layout.fillHeight: true

      value: weight
      onMoved: model.weight = value
    }
  }
}
