import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

GridLayout {
  property var blend
  property bool isFinalBlend

  anchors.margins: 4
  columns: 2
  columnSpacing: 4
  rowSpacing: 0

  Label {
    text: "Name"
    visible: !isFinalBlend
  }
  TextField {
    placeholderText: "(unnamed)"
    text: blend.edit
    visible: !isFinalBlend

    onTextEdited: blend.edit = text
  }
}
