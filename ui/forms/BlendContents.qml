import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

ColumnLayout {
  property int offset: 0

  spacing: 0

  ItemPlaceholder {
    Layout.bottomMargin: Theme.tinyPadding
    Layout.fillWidth: true
    Layout.leftMargin: offset
    Layout.rightMargin: Theme.smallPadding
    Layout.topMargin: Theme.tinyPadding
    text: 'Add pre transform'
  }
  ItemPlaceholder {
    Layout.bottomMargin: Theme.tinyPadding
    Layout.fillWidth: true
    Layout.leftMargin: offset
    Layout.rightMargin: Theme.smallPadding
    Layout.topMargin: Theme.tinyPadding
    text: 'Add formula'
  }
  ItemPlaceholder {
    Layout.bottomMargin: Theme.tinyPadding
    Layout.fillWidth: true
    Layout.leftMargin: offset
    Layout.rightMargin: Theme.smallPadding
    Layout.topMargin: Theme.tinyPadding
    text: 'Add post transform'
  }
}