import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GroupBox {
    id: root
    property alias content: content.children
    signal requested()
    
    Layout.fillWidth: true
    Layout.margins: 10

    ColumnLayout { 
        id: content
        anchors.fill: parent
        spacing: 8
    }

    function popover(sel, type) { 
        root.requested(sel, type)
    }
}