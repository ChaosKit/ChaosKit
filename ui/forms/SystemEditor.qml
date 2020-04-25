import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  spacing: Theme.padding

  StructureEditor {
    Layout.fillWidth: true
  }
  LifetimeEditor {
    Layout.fillWidth: true
  }
}
