import QtQuick 2.0
import QtQuick.Window 2.2
import app.chaoskit 1.0

Rectangle {
    id: root
    color: "#455A64"

    SystemView {
        id: systemview
        objectName: "systemview"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        width: 1024
        height: 1024
        ttl: 20
        scale: 1.0 / Screen.devicePixelRatio
    }
}
