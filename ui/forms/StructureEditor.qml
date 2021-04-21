import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Column {
  id: column
  width: parent.width

  Repeater {
    model: projectModel.system.blends
    delegate: BlendItem {
      width: parent.width
    }
  }

  Button {
    flat: true
    icon.source: Theme.iconUrl('plus')
    icon.width: Theme.smallIconSize
    icon.height: Theme.smallIconSize
    text: 'Add blend'
    width: parent.width
  }
}