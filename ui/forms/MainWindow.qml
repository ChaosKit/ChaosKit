import Qt.labs.platform 1.1
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import ChaosKit 1.0

ApplicationWindow {
  height: 768
  title: `${documentModel.modified ? '*' : ''}${documentModel.name} — ChaosKit`
  visible: true
  width: 1024

  Snackbar {
    id: snackbar
  }

  function openSnackbar(text) {
    snackbar.text = text;
    snackbar.open();
  }

  Connections {
    target: documentModel

    function onIoFailed(error) {
      openSnackbar(error);
    }
    function onModelAboutToBeReset() {
      selectionModel.clearCurrentIndex();
    }
  }

  FileDialog {
    id: openDialog

    acceptLabel: 'Open'
    fileMode: FileDialog.OpenFile
    nameFilters: ["ChaosKit files (*.ck)"]
    options: FileDialog.ReadOnly

    onAccepted: {
      const fileName = Utilities.urlToLocalPath(file);
      documentModel.loadFromFile(fileName);
    }
  }
  FileDialog {
    id: saveDialog

    acceptLabel: 'Save'
    defaultSuffix: 'ck'
    fileMode: FileDialog.SaveFile
    nameFilters: ["ChaosKit files (*.ck)"]

    onAccepted: {
      const fileName = Utilities.urlToLocalPath(file);
      if (documentModel.saveToFile(fileName)) {
        openSnackbar("File saved");
      }
    }
  }
  ExportImageDialog {
    id: exportImageDialog
    onAccepted: {
      const fileName = Utilities.urlToLocalPath(file);
      systemPreview.grabToImage(result => {
        if (result.saveToFile(fileName)) {
          openSnackbar("Image exported");
          console.log("Exported image to", fileName);
        } else {
          openSnackbar("Failed to export image");
          console.error("Could not export image to", fileName);
        }
      });
    }
  }

  MainMenu {
    onNewTriggered: {
      documentModel.resetDocument();
    }
    onOpen: {
      openDialog.open()
    }
    onSave: {
      if (documentModel.filePath === '') {
        saveDialog.open();
      } else if (documentModel.saveToFile(documentModel.filePath)) {
        openSnackbar("File saved");
      }
    }
    onSaveAs: {
      saveDialog.open()
    }
    onExportImage: {
      exportImageDialog.open()
    }
  }

  SystemPreview {
    id: systemPreview
    anchors.centerIn: parent

    gamma: documentModel.documentProxy.gamma
    exposure: documentModel.documentProxy.exposure
    vibrancy: documentModel.documentProxy.vibrancy

    Component.onCompleted: {
      running = true;
    }
  }

  DocumentEditor {
    id: documentEditor
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.bottom: parent.bottom
  }

  Label {
    id: zoomLabel
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.margins: Theme.units(4)
    font.pointSize: Theme.captionFontSize
    font.letterSpacing: Theme.letterSpacing(Theme.captionFontSize)
    font.weight: Font.DemiBold
    text: `Zoom: ${(systemPreview.zoom * 100).toFixed(0)}%`
  }

  Button {
    anchors.bottom: zoomLabel.top
    anchors.left: zoomLabel.left
    anchors.bottomMargin: Theme.padding
    text: "Debug Source"
    outlined: true

    onClicked: {
      astInspectorLoader.active = true;
    }

    Loader {
      id: astInspectorLoader
      active: false
      sourceComponent: AstInspector {
        anchors.centerIn: Overlay.overlay
        modelSource: documentModel.debugSource
        astSource: documentModel.astSource

        onClosed: {
          astInspectorLoader.active = false;
        }
      }
    }
  }

  RowLayout {
    id: actionButtons

    anchors.right: documentEditor.left
    anchors.bottom: parent.bottom
    anchors.margins: Theme.units(4)
    spacing: Theme.units(4)

    Fab {
      id: runButton
      enabled: true
      iconName: systemPreview.running ? "play" : "pause"
      ToolTip.text: "Run/Pause"
      ToolTip.visible: hovered

      onClicked: {
        systemPreview.running = !systemPreview.running;
        console.log("Set value to", isRunning);
      }
    }

    Fab {
      id: randomizeButton
      enabled: true
      iconName: "random-big"
      ToolTip.text: "Randomize the image"
      ToolTip.visible: hovered

      onClicked: {
        documentModel.randomizeSystem();
      }
    }
  }
}
