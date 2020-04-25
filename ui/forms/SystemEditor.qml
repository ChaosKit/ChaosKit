import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  id: root
  spacing: 0

  StructureEditor {
    Layout.fillWidth: true
  }
  FormulaGallery {
    dragContainer: root
    Layout.fillWidth: true
  }
  LifetimeEditor {
    Layout.fillWidth: true
  }
}
