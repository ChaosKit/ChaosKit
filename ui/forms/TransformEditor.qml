import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ChaosKit 1.0

GridLayout {
  id: root
  columns: 3
  columnSpacing: Theme.padding
  rowSpacing: Theme.padding

  property vector2d translation
  property real rotation
  property vector2d scale
  signal translationEdited()
  signal rotationEdited()
  signal scaleEdited()

  Icon {
    iconName: 'move'

    ToolTip.delay: Theme.toolTipDelay
    ToolTip.text: 'Translation'
    ToolTip.visible: hovered
  }
  TextField {
    text: Math.round(root.translation.x * 100) / 100
    validator: DoubleValidator {
      locale: 'en_US'
      notation: DoubleValidator.StandardNotation
    }
    Layout.fillWidth: true

    onEditingFinished: {
      root.translation.x = Number(text);
      root.translationEdited();
    }
  }
  TextField {
    text: Math.round(root.translation.y * 100) / 100
    validator: DoubleValidator {
      locale: 'en_US'
      notation: DoubleValidator.StandardNotation
    }
    Layout.fillWidth: true

    onEditingFinished: {
      root.translation.y = Number(text);
      root.translationEdited();
    }
  }

  Icon {
    iconName: 'scale'

    ToolTip.delay: Theme.toolTipDelay
    ToolTip.text: 'Scale'
    ToolTip.visible: hovered
  }
  TextField {
    text: Math.round(root.scale.x * 100) / 100
    validator: DoubleValidator {
      locale: 'en_US'
      notation: DoubleValidator.StandardNotation
    }
    Layout.fillWidth: true

    onEditingFinished: {
      root.scale.x = Number(text);
      root.scaleEdited();
    }
  }
  TextField {
    text: Math.round(root.scale.y * 100) / 100
    validator: DoubleValidator {
      locale: 'en_US'
      notation: DoubleValidator.StandardNotation
    }
    Layout.fillWidth: true

    onEditingFinished: {
      root.scale.y = Number(text);
      root.scaleEdited();
    }
  }

  Icon {
    iconName: 'rotate'

    ToolTip.delay: Theme.toolTipDelay
    ToolTip.text: 'Rotation'
    ToolTip.visible: hovered
  }
  TextField {
    text: Math.round(root.rotation * 10000) / 10000
    validator: DoubleValidator {
      locale: 'en_US'
      notation: DoubleValidator.StandardNotation
    }
    Layout.columnSpan: 2
    Layout.fillWidth: true

    onEditingFinished: {
      root.rotation = Number(text);
      root.rotationEdited();
    }
  }
}
