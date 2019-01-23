pragma Singleton
import QtQuick 2.0

Item {
  id: fonts

  readonly property FontLoader fontAwesomeRegular: FontLoader {
    source: "./Font Awesome 5 Free-Regular-400.otf"
  }
  readonly property FontLoader fontAwesomeSolid: FontLoader {
    source: "./Font Awesome 5 Free-Solid-900.otf"
  }

  readonly property string icons: fonts.fontAwesomeRegular.name
}
