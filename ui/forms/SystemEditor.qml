import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  spacing: Theme.padding

  readonly property var system: documentModel.systemProxy

  RowLayout {
    spacing: Theme.smallPadding

    Label {
      Layout.leftMargin: Theme.padding
      text: "Lifetime"
    }
    TextField {
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      validator: IntValidator { bottom: -1 }

      onEditingFinished: {
        system.ttl = parseInt(text, 10);
      }
      Component.onCompleted: {
        text = `${system.ttl}`;
      }
    }
  }
}
