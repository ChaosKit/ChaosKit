import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Item {
  id: item
  property var delegateModel
  property var dragContainer
  implicitHeight: layout.implicitHeight

  readonly property var system: documentModel.systemProxy
  readonly property bool isSelected:
      selectionModel.currentIndex === formulaModel.rootIndex

  DropArea {
    id: dropArea
    anchors.fill: parent

    onDropped: {
      if (drop.keys.includes('formula')) {
        documentModel.moveFormulaToBlend(
            drop.source.modelIndex, formulaModel.rootIndex);
      } else if (drop.keys.includes('formulaType')) {
        documentModel.addFormula(
            drop.source.formulaType, formulaModel.rootIndex);
      }
    }
  }

  Rectangle {
    anchors.fill: parent
    color:
        item.isSelected
            ? Theme.alpha(Theme.onSurface, Theme.focusRatio) :
        (hoverHandler.hovered || dropArea.containsDrag)
            ? Theme.alpha(Theme.onSurface, Theme.hoverRatio) :
        'transparent'

    HoverHandler {
      id: hoverHandler
    }
    TapHandler {
      onSingleTapped: {
        selectionModel.setCurrentIndex(
            formulaModel.rootIndex, ItemSelectionModel.ClearAndSelect);
      }
    }

    DelegateModel {
      id: formulaModel
      model: documentModel
      rootIndex: delegateModel.modelIndex(index)
      delegate: Chip {
        id: chip

        property var modelIndex: formulaModel.modelIndex(index)

        text: display
        states: [
          State {
            when: chip.Drag.active
            ParentChange {
              target: chip
              parent: dragContainer
            }
          }
        ]

        Drag.active: dragHandler.active
        Drag.keys: ['formula']
        Drag.hotSpot.x: chip.implicitWidth / 2
        Drag.hotSpot.y: chip.height / 2

        DragHandler {
          id: dragHandler

          onActiveChanged: {
            if (!active) {
              chip.Drag.drop();
            }
          }
        }
      }
    }

    GridLayout {
      id: layout
      columns: 2
      columnSpacing: Theme.padding
      rowSpacing: 0
      x: Theme.padding
      width: item.width - 2 * Theme.padding

      readonly property int topPadding:
          (Theme.chipHeight - Theme.weightIndicatorHeight) / 2
              + Theme.smallPadding

      ColumnRail {
        bottomPadding:
            layout.implicitHeight - layout.topPadding
                - Theme.weightIndicatorHeight
        isFirstItem: item.Positioner.isFirstItem
        isLastItem: item.Positioner.isLastItem
        topPadding: layout.topPadding
        width: Theme.weightIndicatorWidth

        Layout.rowSpan: (display !== '' && formulaModel.count > 0) ? 2 : 1
        Layout.fillHeight: true

        WeightIndicator {
          state:
            (system.isolatedBlendIndex === formulaModel.rootIndex)
                ? 'isolated' :
            (system.isolatedBlendIndex.valid || !model.enabled)
                ? 'disabled' :
            ''
          weight: model.weight || 1
          visible: type === DocumentEntryType.Blend
          y: state === 'editing' ? 2 : layout.topPadding

          onEnabled: {
            system.isolatedBlendIndex = Utilities.invalidModelIndex();
            model.enabled = true;
          }
          onDisabled: {
            system.isolatedBlendIndex = Utilities.invalidModelIndex();
            model.enabled = false;
          }
          onIsolated: {
            system.isolatedBlendIndex = formulaModel.rootIndex;
            model.enabled = true;
          }
          onWeightEdited: {
            model.weight = weight;
          }
        }

        FinalBlendIndicator {
          anchors.horizontalCenter: parent.horizontalCenter
          visible: type === DocumentEntryType.FinalBlend
          y: layout.topPadding
        }
      }

      Label {
        font.pointSize: Theme.captionFontSize
        font.letterSpacing: Theme.letterSpacing(Theme.captionFontSize)
        font.weight: Font.DemiBold
        text: display
        visible: display !== ''
        Layout.alignment: Qt.AlignTop
        Layout.bottomMargin: formulaModel.count > 0 ? 0 : Theme.smallPadding
        Layout.fillWidth: true
        Layout.topMargin: layout.topPadding
      }

      Label {
        color: Theme.onSurfaceDisabled
        font.pointSize: Theme.captionFontSize
        font.letterSpacing: Theme.letterSpacing(Theme.captionFontSize)
        text: "(empty)"
        verticalAlignment: Text.AlignVCenter
        visible: display === '' && formulaModel.count === 0
        Layout.alignment: Qt.AlignTop
        Layout.bottomMargin: Theme.smallPadding
        Layout.fillWidth: true
        Layout.topMargin: layout.topPadding
      }

      Flow {
        id: flow
        spacing: Theme.smallPadding
        visible: formulaModel.count > 0

        Layout.bottomMargin: Theme.smallPadding
        Layout.fillWidth: true
        Layout.minimumHeight: Theme.chipHeight
        Layout.topMargin: Theme.smallPadding

        Repeater {
          model: formulaModel
        }
      }
    }
  }
}
