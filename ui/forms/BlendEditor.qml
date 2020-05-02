import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  id: root
  spacing: 0

  property var currentBlend:
      documentModel.entryAtIndex(selectionModel.currentIndex)

  function updateColoringMethod() {
    for (let i = 0; i < coloringMethodModel.count; i++) {
      const item = coloringMethodModel.get(i);
      if (currentBlend.coloringMethodType === item.value) {
        colorComboBox.currentIndex = i;
        return;
      }
    }
  }
  ListModel {
    id: coloringMethodModel
    ListElement { label: 'No color'; value: 'Noop' }
    ListElement { label: 'Single color'; value: 'SingleColor' }
    ListElement { label: 'Distance-based'; value: 'Distance' }
  }
  Connections {
    target: selectionModel
    onCurrentChanged: {
      if (current.valid) {
        root.updateColoringMethod();

        // Update transform editors
        preEditor.translation = currentBlend.preTranslation;
        preEditor.rotation = currentBlend.preRotation;
        preEditor.scale = currentBlend.preScale;
        postEditor.translation = currentBlend.postTranslation;
        postEditor.rotation = currentBlend.postRotation;
        postEditor.scale = currentBlend.postScale;
      }

      // Clear the formula selection every time the blend selection changes
      // or gets cleared.
      formulaSelectionModel.clearCurrentIndex();
    }
  }

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
    rowSpacing: Theme.padding
    visible: heading.opened && selectionModel.currentIndex.valid

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

    Label {
      text: 'Coloring method'
      Layout.leftMargin: Theme.padding
    }
    ComboBox {
      id: colorComboBox
      model: coloringMethodModel
      textRole: 'label'
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      onActivated: {
        currentBlend.coloringMethodType = coloringMethodModel.get(index).value;
      }
    }

    Label {
      text: 'Color'
      visible: currentBlend.coloringMethodType === 'SingleColor'
      Layout.leftMargin: Theme.padding
    }
    RowLayout {
      spacing: Theme.padding
      visible: currentBlend.coloringMethodType === 'SingleColor'
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      Rectangle {
        color: documentModel.colorAt(currentBlend.coloringMethodParam)
        Layout.preferredWidth: Theme.units(4)
        Layout.preferredHeight: Theme.units(4)
      }

      Slider {
        value: currentBlend.coloringMethodParam || 0
        Layout.fillWidth: true

        onMoved: {
          currentBlend.coloringMethodParam = value;
        }
      }
    }

    Label {
      text: 'Scale'
      visible: currentBlend.coloringMethodType === 'Distance'
      Layout.leftMargin: Theme.padding
    }
    Slider {
      value: currentBlend.coloringMethodParam || 0
      visible: currentBlend.coloringMethodType === 'Distance'
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      onMoved: {
        currentBlend.coloringMethodParam = value;
      }
    }

    GridLayout {
      columns: 2
      columnSpacing: Theme.padding
      rowSpacing: Theme.padding
      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.leftMargin: Theme.padding
      Layout.rightMargin: Theme.padding
      Layout.topMargin: Theme.padding

      Label {
        text: 'Pre'
        font.weight: Font.DemiBold
      }

      Label {
        text: 'Post'
        font.weight: Font.DemiBold
      }

      TransformEditor {
        id: preEditor
        translation: Qt.vector2d(0, 0)
        rotation: 0
        scale: Qt.vector2d(1, 1)
        Layout.fillWidth: true

        onTranslationEdited: {
          currentBlend.preTranslation = translation;
        }
        onRotationEdited: {
          currentBlend.preRotation = rotation;
        }
        onScaleEdited: {
          currentBlend.preScale = scale;
        }
      }
      TransformEditor {
        id: postEditor
        translation: Qt.vector2d(0, 0)
        rotation: 0
        scale: Qt.vector2d(1, 1)
        Layout.fillWidth: true

        onTranslationEdited: {
          currentBlend.postTranslation = translation;
        }
        onRotationEdited: {
          currentBlend.postRotation = rotation;
        }
        onScaleEdited: {
          currentBlend.postScale = scale;
        }
      }
    }

    ScrollView {
      id: scrollView
      clip: true
      contentHeight:
          formulaColumn.visible ? formulaColumn.implicitHeight : 100

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
        text: "This blend has no formulas"
        visible: formulaModel.count === 0
      }
    }

    RowLayout {
      spacing: Theme.smallPadding

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
