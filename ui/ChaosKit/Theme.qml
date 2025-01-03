pragma Singleton
import QtQuick

QtObject {
  // Dimensions

  readonly property int borderRadius: 2
  readonly property int unit: 4
  function units(count) {
    return unit * count;
  }
  readonly property int iconSize: 22
  readonly property int smallIconSize: 16
  readonly property int largePadding: units(4)
  readonly property int padding: units(2)
  readonly property int smallPadding: units(1)
  readonly property int tinyPadding: unit / 2
  readonly property int windowPadding: units(8)
  readonly property int chipHeight: units(4)

  readonly property int weightIndicatorWidth: units(5)
  readonly property int weightIndicatorHeight: units(3)

  // Fonts

  readonly property int headline6FontSize: 14
  readonly property int body1FontSize: 12
  readonly property int buttonFontSize: 10
  readonly property int captionFontSize: 10
  function letterSpacing(fontSize) {
    // Based on Inter Dynamic Metrics: https://rsms.me/inter/dynmetrics/
    const a = -0.0223;
    const b = 0.185;
    const c = -0.1745;
    return (a + b * Math.exp(c * fontSize)) * fontSize;
  }

  readonly property string fontFamily: "Inter"
  // monospaceFont needs to be exposed to the context from the C++ side.
  readonly property string monospaceFontFamily: monospaceFont.family

  // Easing curves

  readonly property var standardCurve: [0.4, 0.0, 0.2, 1.0, 1.0, 1.0]

  // Time

  readonly property int toolTipDelay: 500

  // Icons

  function iconUrl(name) {
    return `qrc:/icons/${name}.svg`
  }

  // Colors

  function alpha(color, alpha) {
    return Qt.rgba(color.r, color.g, color.b, alpha);
  }
  function blend(a, b, ratio) {
    const complement = 1 - ratio;
    return Qt.rgba(
      a.r * complement + b.r * ratio,
      a.g * complement + b.g * ratio,
      a.b * complement + b.b * ratio,
      a.a * complement + b.a * ratio
    );
  }

  function controlColorValue(hovered, pressed, focused) {
    return focused ? 0.12 :
      pressed ? 0.1 :
      hovered ? 0.04 :
      0.0;
  }

  function controlColor(hovered, pressed, focused) {  
    return Qt.rgba(255, 255, 255, controlColorValue(hovered, pressed, focused));
  }

  function buttonColor(control) {
    if (!control.enabled) return Theme.disabledColor;

    const value = controlColorValue(control.hovered, control.down, control.visualFocus);

    if (control.flat) {
      return Qt.rgba(255, 255, 255, value);
    } else {
      return Theme.blend(Theme.primaryColor, Theme.white, value);
    }
  }

  function elevateOpaque(value) {
    return Theme.blend(Theme.surfaceColor, Theme.white, value);
  }

  readonly property color primary900: "#F8791C"
  readonly property color primary800: "#FA9621"
  readonly property color primary700: "#FAA625"
  readonly property color primary600: "#FBB929"
  readonly property color primary500: "#FCC62F"
  readonly property color primary400: "#FCCE3E"
  readonly property color primary300: "#FCD95C"
  readonly property color primary200: "#FDE38A"
  readonly property color primary100: "#FDEEB8"
  readonly property color primary050: "#FEF9E3"

  readonly property color secondary900: "#1D180C"
  readonly property color secondary800: "#3D382D"
  readonly property color secondary700: "#5C574A"
  readonly property color secondary600: "#706A5D"
  readonly property color secondary500: "#999284"
  readonly property color secondary400: "#B9B2A4"
  readonly property color secondary300: "#DDD6C8"
  readonly property color secondary200: "#EEE7D8"
  readonly property color secondary100: "#F8F0E2"
  readonly property color secondary050: "#FFF8E9"

  readonly property real elevation01: 0.05
  readonly property real elevation02: 0.07
  readonly property real elevation03: 0.08
  readonly property real elevation04: 0.09
  readonly property real elevation06: 0.11
  readonly property real elevation08: 0.12
  readonly property real elevation12: 0.14
  readonly property real elevation16: 0.15
  readonly property real elevation24: 0.16

  readonly property color surfaceColor: "#121212"
  readonly property color backgroundColor: surfaceColor
  readonly property color errorColor: "#CF6679"
  readonly property color primaryColor: primary200
  readonly property color primaryVariantColor: primary900
  readonly property color secondaryColor: secondary200
  readonly property color borderColor: Qt.rgba(1, 1, 1, 0.12)
  readonly property color disabledColor: Qt.rgba(1, 1, 1, 0.12)
  readonly property color surfaceOverlay: Qt.rgba(1, 1, 1, 0.12)

  readonly property real hoverRatio: 0.04
  readonly property real focusRatio: 0.12
  readonly property real borderRatio: 0.12

  readonly property color black: "black"
  readonly property color white: "white"

  readonly property color onPrimary: "black"
  readonly property color onSurface: "white"

  readonly property color onPrimaryHigh: onPrimary
  readonly property color onPrimaryMedium: alpha(onPrimary, 0.74)
  readonly property color onPrimaryDisabled: alpha(onPrimary, 0.38)

  readonly property color onSurfaceHigh: alpha(onSurface, 0.87)
  readonly property color onSurfaceMedium: alpha(onSurface, 0.60)
  readonly property color onSurfaceDisabled: alpha(onSurface, 0.38)
}
