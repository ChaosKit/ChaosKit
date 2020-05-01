import QtQml.Models 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

Rectangle {
  id: item

  property bool isSelected: false
  signal clicked()

  color:
      item.isSelected
          ? Theme.alpha(Theme.onSurface, Theme.focusRatio) :
      hoverHandler.hovered
          ? Theme.alpha(Theme.onSurface, Theme.hoverRatio) :
      'transparent'
  implicitHeight: layout.implicitHeight

  HoverHandler {
    id: hoverHandler
  }
  TapHandler {
    onTapped: {
      item.clicked();
    }
  }

  RowLayout {
    id: layout
    spacing: Theme.padding
    width: item.width - 2 * Theme.padding
    x: Theme.padding

    ColumnRail {
      bottomPadding: Theme.smallPadding
      isFirstItem: item.Positioner.isFirstItem
      isLastItem: item.Positioner.isLastItem
      topPadding: Theme.smallPadding

      Layout.fillHeight: true
      Layout.preferredWidth: Theme.weightIndicatorWidth
      Layout.preferredHeight: Theme.weightIndicatorHeight + 2 * Theme.smallPadding

      WeightIndicator {
        anchors.centerIn: parent
        weight: model.weight || 1

        onWeightEdited: {
          model.weight = weight;
        }
      }
    }

    Label {
      font.pointSize: Theme.captionFontSize
      font.letterSpacing: Theme.letterSpacing(Theme.captionFontSize)
      font.weight: Font.DemiBold
      text: display
      Layout.fillWidth: true
    }
  }
}
