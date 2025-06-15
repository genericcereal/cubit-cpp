import QtQuick
import Cubit 1.0
import Cubit.UI 1.0

Item {
    id: root
    
    property var creationDragHandler
    property var controller
    property ViewportCache viewportCache
    
    // Element creation preview
    Rectangle {
        id: creationPreview
        visible: creationDragHandler && creationDragHandler.active && controller && controller.mode !== "select"
        color: Config.elementCreationPreviewColor
        border.color: Config.elementCreationPreviewBorderColor
        border.width: Config.elementCreationPreviewBorderWidth
        
        property var creationRect: creationDragHandler && creationDragHandler.active ? creationDragHandler.getCreationRect() : Qt.rect(0, 0, 0, 0)
        
        x: viewportCache ? viewportCache.canvasXToRelative(creationRect.x) : 0
        y: viewportCache ? viewportCache.canvasYToRelative(creationRect.y) : 0
        width: creationRect.width
        height: creationRect.height
    }
}