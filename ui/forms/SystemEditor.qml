import QtQml.Models 2.15
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ScrollView {
  ColumnLayout {
    spacing: 0
    width: parent.width

    StructureEditor {
      Layout.fillWidth: true
    }
  }
}
