import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Popup {
  id: popup

  signal formulaPicked(string formulaType)

  padding: Theme.smallPadding
  height: 200
  width: 240

  ScrollView {
    id: scrollView
    anchors.fill: parent
    clip: true
    contentWidth: availableWidth
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    GridView {
      id: grid
      cellWidth: 48 + Theme.smallPadding * 2
      cellHeight: 48 + Theme.smallPadding * 2
      model: formulaList
      width: parent.width

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

        HoverHandler {
          id: hoverHandler
        }
        TapHandler {
          onSingleTapped: {
            popup.formulaPicked(formulaType);
            popup.close();
          }
        }

        Image {
          anchors.fill: parent
          anchors.margins: Theme.smallPadding
          source: 'image://formula/' + modelData
          sourceSize.width: grid.cellWidth
          sourceSize.height: grid.cellHeight

          ToolTip.text: modelData
          ToolTip.visible: hovered
        }
      }
    }
  }
}
