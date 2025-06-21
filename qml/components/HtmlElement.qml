import QtQuick
import QtQuick.Controls
import QtWebEngineQuick
import Cubit 1.0
import "."

DesignElement {
    id: root
    
    property Html htmlElement: element as Html
    
    // WebEngineView for HTML content
    WebEngineView {
        id: webView
        anchors.fill: parent
        
        // Settings for better integration
        settings.localContentCanAccessRemoteUrls: true
        settings.javascriptEnabled: true
        
        Component.onCompleted: {
            if (htmlElement) {
                loadContent()
            }
        }
        
        Connections {
            target: htmlElement
            
            function onUrlChanged() {
                loadContent()
            }
            
            function onHtmlChanged() {
                loadContent()
            }
        }
        
        function loadContent() {
            if (!htmlElement) return
            
            if (htmlElement.url && htmlElement.url.toString() !== "") {
                // Load URL
                webView.url = htmlElement.url
            } else if (htmlElement.html && htmlElement.html !== "") {
                // Load HTML content
                webView.loadHtml(htmlElement.html)
            }
        }
        
        // Disable navigation for embedded content
        onNavigationRequested: (request) => {
            if (request.navigationType !== WebEngineView.NavigationTypeTyped) {
                request.action = WebEngineView.IgnoreRequest
            }
        }
    }
}