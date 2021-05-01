import QtQuick 2.15
import QtQuick.Controls 2.15

SpinBox {
  id: root

  property int precision: 2
  readonly property int factor: Math.pow(10, precision) | 0
  property real realValue: value / factor

  editable: true
  from: 0
  to: 100 * factor
  stepSize: factor
  value: factor

  validator: DoubleValidator {
    bottom: Math.min(root.min, root.max)
    top: Math.max(root.min, root.max)
  }

  textFromValue: function(value, locale) {
    return Number(value / root.factor).toLocaleString(locale, 'f', root.precision);
  }
  valueFromText: function(text, locale) {
    return Number.fromLocaleString(locale, text) * root.factor | 0;
  }
}
