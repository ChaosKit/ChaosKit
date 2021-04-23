import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ChaosKit 1.0

ColumnLayout {
  id: root

  property var blend
  property int offset: 0

  spacing: 0

  Component {
    id: preItem
    TransformItem {
      name: 'Pre transform'
      offset: root.offset
      transformModel: blend.pre
    }
  }
  Loader {
    Layout.fillWidth: true
    // The tinyPadding at the end is to make the size equal to the placeholder.
    Layout.preferredHeight:
        Theme.smallIconSize + Theme.smallPadding * 2 + Theme.tinyPadding * 2
    active: !!blend.pre
    sourceComponent: preItem
    visible: !!blend.pre
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
    TransformItem {
      name: 'Post transform'
      offset: root.offset
      transformModel: blend.post
    }
  }
  Loader {
    Layout.fillWidth: true
    // The tinyPadding at the end is to make the size equal to the placeholder.
    Layout.preferredHeight:
        Theme.smallIconSize + Theme.smallPadding * 2 + Theme.tinyPadding * 2
    active: !!blend.post
    sourceComponent: postItem
    visible: !!blend.post
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
