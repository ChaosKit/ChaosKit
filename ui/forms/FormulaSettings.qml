import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChaosKit

ScrollView {
  id: root

  property alias blend: list.blend

  clip: true
  contentWidth: availableWidth


  ColumnLayout {
    spacing: 0
    width: parent.width

    Heading {
      Layout.leftMargin: Theme.padding
      Layout.topMargin: Theme.padding
      text: 'Formulas'
    }

    FormulaList {
      id: list
      Layout.fillWidth: true
    }
  }
}
