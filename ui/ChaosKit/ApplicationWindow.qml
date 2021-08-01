import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Templates as T
import ChaosKit

T.ApplicationWindow {
    color: Theme.backgroundColor
    font.family: Theme.fontFamily
    font.pointSize: Theme.body1FontSize

    Overlay.modal: Rectangle {
        color: Color.transparent(window.palette.shadow, 0.5)
    }

    Overlay.modeless: Rectangle {
        color: Color.transparent(window.palette.shadow, 0.12)
    }
}
