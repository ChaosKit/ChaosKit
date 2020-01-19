import Qt.labs.platform 1.1
import QtQuick 2.12

FileDialog {
  acceptLabel: 'Export'
  defaultSuffix:
    selectedNameFilter.extensions[0] === '*' ?
    "png" :
    selectedNameFilter.extensions[0]
  fileMode: FileDialog.SaveFile
  nameFilters: exportFormats
  selectedNameFilter.index: defaultExportFormat
}
