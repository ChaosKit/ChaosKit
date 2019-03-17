import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import "../controls"
import "../resources"

RowLayout {
  id: itemRoot
  spacing: 0

  property bool itemEnabled: true  // TODO: use the model
  property var rootModel: null

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

  SymbolButton {
    symbol: Icons.faTrashAlt
    onClicked: rootModel.removeFormula(blendIndex, index)
  }
}
