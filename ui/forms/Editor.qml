import QtQuick 2.0
import QtQuick.Window 2.2
import app.chaoskit 1.0

Item {
    property bool autoRefresh: true
    property int refreshInterval: 100
    property alias running: systemView.running
    property alias system: systemView.system
    property alias gamma: systemView.gamma
    property alias exposure: systemView.exposure
    property alias vibrancy: systemView.vibrancy
    property alias ttl: systemView.ttl

    SystemView {
        id: systemView
        objectName: "systemview"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        width: 1024
        height: 1024
        ttl: 20
        scale: 1.0 / Screen.devicePixelRatio
    }

    Timer {
        interval: parent.refreshInterval
        running: parent.autoRefresh && systemView.running
        repeat: true
        onTriggered: systemView.update();
    }
}
