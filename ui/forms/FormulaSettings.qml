import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

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
