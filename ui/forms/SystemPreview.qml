import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12
import ChaosKit 1.0

SystemView {
  id: systemView

  property bool autoRefresh: true
  property int refreshInterval: 100

  colorMap: documentModel.documentProxy.colorMap
  colorMapRegistry: globalColorMapRegistry
  height: documentModel.documentProxy.height
  model: documentModel
  scale: 1.0 / Screen.devicePixelRatio
  ttl: 20
  width: documentModel.documentProxy.width

  Timer {
    interval: parent.refreshInterval
    running: parent.autoRefresh && parent.running
    repeat: true
    onTriggered: {
      systemView.update();
    }
  }
}
