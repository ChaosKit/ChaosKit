import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  id: root
  spacing: 0

  property var blend

  function updateColoringMethod() {
    for (let i = 0; i < coloringMethodModel.count; i++) {
      const item = coloringMethodModel.get(i);
      if (blend.coloringMethodType === item.value) {
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

    function onCurrentChanged(current, previous) {
      if (current.valid) {
        root.updateColoringMethod();

        // Update transform editors
        preEditor.translation = blend.preTranslation;
        preEditor.rotation = blend.preRotation;
        preEditor.scale = blend.preScale;
        postEditor.translation = blend.postTranslation;
        postEditor.rotation = blend.postRotation;
        postEditor.scale = blend.postScale;
      }

      // Clear the formula selection every time the blend selection changes
      // or gets cleared.
      formulaSelectionModel.clearCurrentIndex();
    }
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
      visible: blend.type === DocumentEntryType.Blend
      Layout.leftMargin: Theme.padding
    }
    TextField {
      text: blend.edit || ''
      visible: blend.type === DocumentEntryType.Blend
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      onEditingFinished: {
        blend.edit = text;
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
        blend.coloringMethodType = coloringMethodModel.get(index).value;
      }
    }

    Label {
      text: 'Color'
      visible: blend.coloringMethodType === 'SingleColor'
      Layout.leftMargin: Theme.padding
    }
    RowLayout {
      spacing: Theme.padding
      visible: blend.coloringMethodType === 'SingleColor'
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      Rectangle {
        color: documentModel.colorAt(blend.coloringMethodParam)
        Layout.preferredWidth: Theme.units(4)
        Layout.preferredHeight: Theme.units(4)
      }

      Slider {
        value: blend.coloringMethodParam || 0
        Layout.fillWidth: true

        onMoved: {
          blend.coloringMethodParam = value;
        }
      }
    }

    Label {
      text: 'Scale'
      visible: blend.coloringMethodType === 'Distance'
      Layout.leftMargin: Theme.padding
    }
    Slider {
      value: blend.coloringMethodParam || 0
      visible: blend.coloringMethodType === 'Distance'
      Layout.fillWidth: true
      Layout.rightMargin: Theme.padding

      onMoved: {
        blend.coloringMethodParam = value;
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

      RowLayout {
        Layout.fillWidth: true

        Label {
          text: 'Pre'
          font.weight: Font.DemiBold
          Layout.fillWidth: true
        }

        IconButton {
          iconName: 'undo'

          onClicked: {
            blend.preTranslation = Qt.vector2d(0, 0);
            blend.preScale = Qt.vector2d(1, 1);
            blend.preRotation = 0;
            preEditor.translation = blend.preTranslation;
            preEditor.rotation = blend.preRotation;
            preEditor.scale = blend.preScale;
          }
        }
      }

      RowLayout {
        Layout.fillWidth: true

        Label {
          text: 'Post'
          font.weight: Font.DemiBold
          Layout.fillWidth: true
        }

        IconButton {
          iconName: 'undo'

          onClicked: {
            blend.postTranslation = Qt.vector2d(0, 0);
            blend.postScale = Qt.vector2d(1, 1);
            blend.postRotation = 0;
            postEditor.translation = blend.postTranslation;
            postEditor.rotation = blend.postRotation;
            postEditor.scale = blend.postScale;
          }
        }
      }

      TransformEditor {
        id: preEditor
        translation: Qt.vector2d(0, 0)
        rotation: 0
        scale: Qt.vector2d(1, 1)
        Layout.fillWidth: true

        onTranslationEdited: {
          blend.preTranslation = translation;
        }
        onRotationEdited: {
          blend.preRotation = rotation;
        }
        onScaleEdited: {
          blend.preScale = scale;
        }
      }
      TransformEditor {
        id: postEditor
        translation: Qt.vector2d(0, 0)
        rotation: 0
        scale: Qt.vector2d(1, 1)
        Layout.fillWidth: true

        onTranslationEdited: {
          blend.postTranslation = translation;
        }
        onRotationEdited: {
          blend.postRotation = rotation;
        }
        onScaleEdited: {
          blend.postScale = scale;
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
