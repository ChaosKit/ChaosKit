pragma Singleton
import QtQuick 2.12

QtObject {
  // Dimensions

  readonly property int borderRadius: 2
  readonly property int unit: 4
  function units(count) {
    return unit * count;
  }
  readonly property int iconSize: 22
  readonly property int smallIconSize: 16
  readonly property int padding: units(2)
  readonly property int smallPadding: units(1)

  // Fonts

  readonly property int headline6FontSize: 14
  readonly property int body1FontSize: 12
  readonly property int buttonFontSize: 12
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

  readonly property color surfaceColor: "#121212"
  readonly property color backgroundColor: surfaceColor
  readonly property color errorColor: "#CF6679"
  readonly property color primaryColor: primary200
  readonly property color primaryVariantColor: primary900
  readonly property color secondaryColor: secondary200
  readonly property color borderColor: Qt.rgba(1, 1, 1, 0.12)
  readonly property color disabledColor: Qt.rgba(1, 1, 1, 0.12)
  readonly property color surfaceOverlay: Qt.rgba(1, 1, 1, 0.12)

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
