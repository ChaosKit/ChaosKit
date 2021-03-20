import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

GridLayout {
//  readonly property var document: documentModel.documentProxy

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

    validator: IntValidator { bottom: 0 }
    onEditingFinished: {
//      document.width = parseInt(text, 10);
    }
    Component.onCompleted: {
//      text = `${document.width}`;
    }
  }
  Label {
    text: "x"
  }
  TextField {
    Layout.fillWidth: true
    Layout.rightMargin: Theme.padding

    validator: IntValidator { bottom: 0 }
    onEditingFinished: {
//      document.height = parseInt(text, 10);
    }
    Component.onCompleted: {
//      text = `${document.height}`;
    }
  }
}
