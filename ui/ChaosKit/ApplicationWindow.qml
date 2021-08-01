import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import ChaosKit 1.0

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
