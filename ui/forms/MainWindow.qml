import Qt.labs.platform
import QtQml
import QtQuick
import QtQuick.Controls
import ChaosKit

ApplicationWindow {
  height: 768
  title: `${projectModel.modified ? '*' : ''}${projectModel.name} — ChaosKit`
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
    target: projectModel

    function onFileIoFailed(error) {
      openSnackbar(error);
    }
  }

  FileDialog {
    id: openDialog

    acceptLabel: 'Open'
    fileMode: FileDialog.OpenFile
    nameFilters: ["ChaosKit files (*.ck)"]
    options: FileDialog.ReadOnly

    onAccepted: {
      projectModel.loadFromUrl(file);
    }
  }
  FileDialog {
    id: saveDialog

    acceptLabel: 'Save'
    defaultSuffix: 'ck'
    fileMode: FileDialog.SaveFile
    nameFilters: ["ChaosKit files (*.ck)"]

    onAccepted: {
      if (projectModel.saveToUrl(file)) {
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
      projectModel.reset();
    }
    onOpen: {
      openDialog.open()
    }
    onSave: {
      if (projectModel.fileUrl == "") {
        saveDialog.open();
      } else if (projectModel.saveToUrl(projectModel.fileUrl)) {
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

    Component.onCompleted: {
     running = true;
    }
  }

  ProjectEditor {
    id: projectEditor
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
  
  OutlinedButton {
    id: debugSourceButton

    anchors.bottom: zoomLabel.top
    anchors.left: zoomLabel.left
    anchors.bottomMargin: Theme.padding

    text: "Debug Source"

    onClicked: {
      astInspectorLoader.active = true;
    }

    Loader {
      id: astInspectorLoader
      active: false
      sourceComponent: AstInspector {
        anchors.centerIn: Overlay.overlay
        modelSource: projectModel.system.modelSource
        astSource: projectModel.system.astSource

        onClosed: {
          astInspectorLoader.active = false;
        }
      }
    }
  }

  OutlinedButton {
    id: pauseButton

    anchors.bottom: zoomLabel.top
    anchors.left: debugSourceButton.right
    anchors.bottomMargin: Theme.padding
    anchors.leftMargin: Theme.padding

    text: systemPreview.running ? "Pause" : "Run"
    icon.source: systemPreview.running ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"

    onClicked: {
      systemPreview.running = !systemPreview.running
    }
  }
}
