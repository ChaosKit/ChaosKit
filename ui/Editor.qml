import QtQuick 2.0
import QtQuick.Window 2.2
import app.chaoskit 1.0

Rectangle {
    id: root
    color: "#455A64"

    SystemView {
        id: systemview
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        width: 512
        height: 512
        ttl: 20
        scale: 1.0 / Screen.devicePixelRatio
    }
}
