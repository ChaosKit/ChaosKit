import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
  height: 768
  title: "ChaosKit"
  visible: true
  width: 1024

  SystemPreview {
    anchors.centerIn: parent

    Component.onCompleted: {
      running = true;
    }
  }
}
