import QtQuick
import Cubit

Item {
    id: root
    
    property string fontFamily: ""
    property string fontWeight: "regular"
    property bool isLoaded: false
    
    signal loaded()
    signal error(string message)
    
    onFontFamilyChanged: updateLoadState()
    onFontWeightChanged: updateLoadState()
    
    function updateLoadState() {
        if (fontFamily) {
            isLoaded = GoogleFonts.isFontLoaded(fontFamily, fontWeight)
            if (isLoaded) {
                loaded()
            } else {
                loadFont()
            }
        }
    }
    
    function loadFont() {
        if (!fontFamily) {
            return
        }
        
        if (GoogleFonts.isFontLoaded(fontFamily, fontWeight)) {
            isLoaded = true
            loaded()
            return
        }
        
        // Load the font with specific weight
        GoogleFonts.loadFont(fontFamily, fontWeight)
    }
    
    Connections {
        target: GoogleFonts
        function onFontLoaded(loadedFamily) {
            if (loadedFamily === root.fontFamily) {
                // Check if our specific weight is loaded
                if (GoogleFonts.isFontLoaded(root.fontFamily, root.fontWeight)) {
                    root.isLoaded = true
                    root.loaded()
                }
            }
        }
        
        function onFontLoadError(errorFamily, errorMessage) {
            if (errorFamily === root.fontFamily) {
                root.error(errorMessage)
            }
        }
    }
    
    Component.onCompleted: {
        updateLoadState()
    }
}