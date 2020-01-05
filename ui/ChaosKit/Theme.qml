pragma Singleton
import QtQuick 2.12

QtObject {
  // Dimensions

  readonly property int unit: 4
  function units(count) {
    return unit * count;
  }

  // Fonts

  readonly property string fontFamily: "Inter"
  // monospaceFont needs to be exposed to the context from the C++ side.
  readonly property string monospaceFontFamily: monospaceFont.family

  // Colors

  function alpha(color, alpha) {
    return Qt.rgba(color.r, color.g, color.b, alpha);
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
  readonly property color borderColor: alpha("white", 0.12)

  readonly property color onPrimary: "black"
  readonly property color onSurface: "white"

  readonly property color onPrimaryHigh: onPrimary
  readonly property color onPrimaryMedium: alpha(onPrimary, 0.74)
  readonly property color onPrimaryDisabled: alpha(onPrimary, 0.38)

  readonly property color onSurfaceHigh: alpha(onSurface, 0.87)
  readonly property color onSurfaceMedium: alpha(onSurface, 0.60)
  readonly property color onSurfaceDisabled: alpha(onSurface, 0.38)
}
