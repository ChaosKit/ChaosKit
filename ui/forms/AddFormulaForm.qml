import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import "../controls"
import "../resources"

RowLayout {
  spacing: 0

  property var blendIndex: null

  SymbolButton {
    symbol: Icons.faPlus
    onClicked: documentModel.addFormula(formulaTypePicker.currentText, blendIndex)
  }

  ComboBox {
    id: formulaTypePicker
    Layout.fillWidth: true
    Layout.fillHeight: true
    model: formulaList
  }

  Item {
    Layout.preferredWidth: 80
    Layout.fillHeight: true

    Label {
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.verticalCenter: parent.verticalCenter
      anchors.horizontalCenterOffset: 10
      font.pixelSize: 9
      font.letterSpacing: 0.45
      text: "1.0"
    }
  }

  SymbolButton {
    symbol: Icons.faLock
  }
}
