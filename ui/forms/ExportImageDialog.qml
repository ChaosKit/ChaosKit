import Qt.labs.platform
import QtQuick

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
