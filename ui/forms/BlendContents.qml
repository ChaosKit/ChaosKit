import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

ColumnLayout {
  property var blend
  property int offset: 0

  spacing: 0

  Component {
    id: preItem
    Label {
      text: 'Pre transform'
    }
  }

  Loader {
    active: !!blend.pre
    sourceComponent: preItem
  }
  ItemPlaceholder {
    Layout.bottomMargin: Theme.tinyPadding
    Layout.fillWidth: true
    Layout.leftMargin: offset
    Layout.rightMargin: Theme.smallPadding
    Layout.topMargin: Theme.tinyPadding
    text: 'Add pre transform'
    visible: !blend.pre

    onClicked: {
      blend.addPre();
    }
  }

  ItemPlaceholder {
    Layout.bottomMargin: Theme.tinyPadding
    Layout.fillWidth: true
    Layout.leftMargin: offset
    Layout.rightMargin: Theme.smallPadding
    Layout.topMargin: Theme.tinyPadding
    text: 'Add formula'
  }

  Component {
    id: postItem
    Label {
      text: 'Post transform'
    }
  }

  Loader {
    active: !!blend.post
    sourceComponent: postItem
  }
  ItemPlaceholder {
    Layout.bottomMargin: Theme.tinyPadding
    Layout.fillWidth: true
    Layout.leftMargin: offset
    Layout.rightMargin: Theme.smallPadding
    Layout.topMargin: Theme.tinyPadding
    text: 'Add post transform'
    visible: !blend.post

    onClicked: {
      blend.addPost();
    }
  }
}
