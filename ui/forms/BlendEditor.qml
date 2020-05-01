import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  spacing: 0

  property var currentBlend:
      documentModel.entryAtIndex(selectionModel.currentIndex)

  CollapsibleHeading {
    id: heading
    opened: true
    text: 'Selected Blend'
  }

  Label {
    color: Theme.onSurfaceDisabled
    horizontalAlignment: Text.AlignHCenter
    padding: Theme.padding
    text: 'Nothing selected'
    visible: heading.opened && !selectionModel.currentIndex.valid
    Layout.fillWidth: true
  }

  GridLayout {
    columns: 2
    columnSpacing: Theme.padding
    rowSpacing: Theme.smallPadding
    visible: heading.opened

    Layout.bottomMargin: Theme.smallPadding
    Layout.fillWidth: true

    Label {
      text: 'Name'
      visible: currentBlend.type === DocumentEntryType.Blend
      Layout.leftMargin: Theme.padding
    }
    TextField {
      text: currentBlend.edit || ''
      visible: currentBlend.type === DocumentEntryType.Blend
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      onEditingFinished: {
        currentBlend.edit = text;
      }
    }

    ScrollView {
      id: scrollView
      clip: true
      contentHeight:
          formulaColumn.visible ? formulaColumn.implicitHeight : 100
      visible: selectionModel.currentIndex.valid

      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.preferredHeight: 100

      Component.onCompleted: {
        // Disable flicking in the inner Flickable.
        // scrollView.contentItem.interactive = false;
        scrollView.contentItem.boundsBehavior = Flickable.StopAtBounds;
      }

      ItemSelectionModel {
        id: formulaSelectionModel
        model: documentModel
      }

      // Clear the formula selection every time the blend selection changes
      // or gets cleared.
      Connections {
        target: selectionModel

        onCurrentChanged: {
          formulaSelectionModel.clearCurrentIndex();
        }
      }

      Column {
        id: formulaColumn
        visible: formulaModel.count > 0

        Repeater {
          model: DelegateModel {
            id: formulaModel
            model: documentModel
            rootIndex: selectionModel.currentIndex

            delegate: FormulaListItem {
              readonly property var modelIndex: formulaModel.modelIndex(index)

              isSelected: formulaSelectionModel.currentIndex === modelIndex
              width: scrollView.width

              onClicked: {
                formulaSelectionModel.setCurrentIndex(
                    modelIndex, ItemSelectionModel.ClearAndSelect);
              }
            }
          }
        }
      }

      Label {
        anchors.fill: parent
        color: Theme.onSurfaceDisabled
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        // font.pointSize: Theme.captionFontSize
        // font.letterSpacing: Theme.letterSpacing(Theme.captionFontSize)
        text: "This blend has no formulas"
        visible: formulaModel.count === 0
      }
    }

    RowLayout {
      spacing: Theme.smallPadding
      visible: selectionModel.currentIndex.valid

      Layout.columnSpan: 2
      Layout.leftMargin: Theme.padding
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      IconButton {
        iconName: 'plus'

        ToolTip.delay: Theme.toolTipDelay
        ToolTip.text: 'Add formula'
        ToolTip.visible: hovered

        onClicked: {
          formulaPopup.open();
        }

        FormulaPopup {
          id: formulaPopup
          onFormulaPicked: {
            documentModel.addFormula(formulaType, selectionModel.currentIndex);
          }
        }
      }
      IconButton {
        enabled: formulaSelectionModel.currentIndex.valid
        iconName: 'random'

        ToolTip.delay: Theme.toolTipDelay
        ToolTip.text: 'Randomize parameters'
        ToolTip.visible: hovered

        onClicked: {
          documentModel.randomizeParams(formulaSelectionModel.currentIndex);
        }
      }
      Item {
        Layout.fillWidth: true
      }
      IconButton {
        enabled: formulaSelectionModel.currentIndex.valid
        iconName: 'trash'

        ToolTip.delay: Theme.toolTipDelay
        ToolTip.text: 'Remove formula'
        ToolTip.visible: hovered

        onClicked: {
          documentModel.removeRowAtIndex(formulaSelectionModel.currentIndex);
        }
      }
    }
  }
}
