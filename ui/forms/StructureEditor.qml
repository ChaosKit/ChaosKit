import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  spacing: 0

  // Changing the rootIndex on DelegateModel doesn't work, it gets reset to the
  // model's root. Fortunately reloading the component helps.
  Connections {
    target: documentModel
    onSystemIndexChanged: {
      loader.sourceComponent = undefined;
      loader.sourceComponent = blendColumn;
    }
  }

  Heading {
    padding: Theme.padding
    text: 'Structure'
  }

  Component {
    id: blendColumn
    Column {
      Repeater {
        model: DelegateModel {
          id: systemModel
          model: documentModel
          rootIndex: documentModel.systemIndex
          delegate: BlendListItem {
            delegateModel: systemModel
            dragContainer: root
            width: root.width
          }
        }
      }
    }
  }

  ScrollView {
    id: root
    clip: true

    Layout.fillWidth: true
    Layout.preferredHeight: 100

    Component.onCompleted: {
      // Disable flicking in the inner Flickable.
      // root.contentItem.interactive = false;
      root.contentItem.boundsBehavior = Flickable.StopAtBounds;
    }

    Loader {
      id: loader
      sourceComponent: blendColumn
    }
  }


  RowLayout {
    Layout.fillWidth: true
    Layout.leftMargin: Theme.padding
    Layout.rightMargin: Theme.padding

    IconButton {
      iconName: 'plus'

      onClicked: {
        documentModel.addBlend();
      }
    }

    Item {
      Layout.fillWidth: true
    }

    IconButton {
      enabled:
          selectionModel.currentIndex.valid
              && !documentModel.isFinalBlend(selectionModel.currentIndex)
      iconName: 'trash'

      onClicked: {
        documentModel.removeRowAtIndex(selectionModel.currentIndex);
      }
    }
  }
}
