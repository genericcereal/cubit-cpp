import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    property alias label: label.text
    property alias delegate: container.children
    spacing: 8

    Label { 
        id: label
        Layout.preferredWidth: 80
    }
    
    Item { 
        id: container
        Layout.fillWidth: true
        Layout.preferredHeight: childrenRect.height
    }
}