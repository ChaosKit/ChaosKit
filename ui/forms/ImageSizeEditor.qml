import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

GridLayout {
  columns: 3
  columnSpacing: Theme.padding
  rowSpacing: Theme.smallPadding

  Heading {
    Layout.columnSpan: 3
    Layout.fillWidth: true

    padding: Theme.padding
    text: "Image Size"
  }

  TextField {
    Layout.fillWidth: true
    Layout.leftMargin: Theme.padding

    inputMethodHints: Qt.ImhDigitsOnly
    validator: IntValidator { bottom: 0 }
    onEditingFinished: {
      projectModel.width = parseInt(text, 10);
    }
    Component.onCompleted: {
      text = String(projectModel.width);
    }
  }
  Label {
    text: "x"
  }
  TextField {
    Layout.fillWidth: true
    Layout.rightMargin: Theme.padding

    inputMethodHints: Qt.ImhDigitsOnly
    validator: IntValidator { bottom: 0 }
    onEditingFinished: {
      projectModel.height = parseInt(text, 10);
    }
    Component.onCompleted: {
      text = String(projectModel.height);
    }
  }
}
