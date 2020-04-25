import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

ColumnLayout {
  property var dragContainer

  spacing: 0

  CollapsibleHeading {
    id: heading
    text: "Formula Gallery"
  }

  ScrollView {
    id: root
    clip: true
    visible: heading.opened

    Layout.fillWidth: true
    Layout.preferredHeight: 100

    Component.onCompleted: {
      // Disable flicking in the inner Flickable.
      // root.contentItem.interactive = false;
      root.contentItem.boundsBehavior = Flickable.StopAtBounds;
    }

    GridView {
      id: grid
      cellWidth: 48 + Theme.smallPadding * 2
      cellHeight: 48 + Theme.smallPadding * 2
      currentIndex: -1
      delegate: Rectangle {
        id: formula

        property string formulaType: modelData
        property alias hovered: hoverHandler.hovered

        color: Theme.alpha(
            Theme.onSurface,
            GridView.isCurrentItem ? Theme.focusRatio :
            hovered ? Theme.hoverRatio :
            0
        )
        width: grid.cellWidth
        height: grid.cellHeight

        Drag.active: dragHandler.active
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.keys: ['formulaType']

        HoverHandler {
          id: hoverHandler
        }
        TapHandler {
          onSingleTapped: {
            grid.currentIndex = index;
          }
        }
        DragHandler {
          id: dragHandler

          property real startX: formula.x
          property real startY: formula.y
          property var startParent: formula.parent

          onActiveChanged: {
            if (active) {
              startX = formula.x;
              startY = formula.y;
              startParent = formula.parent;
              formula.parent = dragContainer;
            } else {
              formula.Drag.drop();
              formula.parent = startParent;
              formula.x = startX;
              formula.y = startY;
            }
          }
        }

        Image {
          anchors.fill: parent
          anchors.margins: Theme.smallPadding
          source: 'image://formula/' + modelData

          ToolTip.text: modelData
          ToolTip.visible: hovered && !formula.Drag.active
        }
      }
      model: formulaList
      width: root.width
    }
  }

  RowLayout {
    spacing: Theme.smallPadding
    visible: heading.opened
    Layout.fillWidth: true
    Layout.topMargin: Theme.padding
    Layout.leftMargin: Theme.padding
    Layout.rightMargin: Theme.padding

    Button {
      enabled: grid.currentIndex >= 0
      flat: true
      icon.source: Theme.iconUrl('plus')
      text: 'Add blend'

      onClicked: {
        documentModel.addBlend(grid.currentItem.formulaType);
      }
    }

    Button {
      enabled: grid.currentIndex >= 0 && selectionModel.currentIndex.valid
      flat: true
      icon.source: Theme.iconUrl('plus')
      text: 'Add to selected'

      onClicked: {
        documentModel.addFormula(
            grid.currentItem.formulaType, selectionModel.currentIndex);
      }
    }
  }
}
