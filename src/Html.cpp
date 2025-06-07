#include "Html.h"
#include <QWebEngineView>
#include <QResizeEvent>
#include <QDebug>

Html::Html(int id, QWidget *parent) : Element(ElementType::HtmlType, id, parent), webView(nullptr) {
    htmlName = QString("Html_%1").arg(elementId, 2, 10, QChar('0'));
    // Don't set fixed size - let it expand to fill parent
    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("QFrame { background-color: transparent; border: none; }");
    
    setObjectName(htmlName);
    
    // Create the web view
    webView = new QWebEngineView(this);
    
    // Set default HTML content
    content = "<html><body><h1>HTML Element</h1></body></html>";
    webView->setHtml(content);
    
    // IMPORTANT: Html elements should not handle mouse events directly
    // Mouse events are handled by the associated ClientRect
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    // Make the web view also transparent for mouse events
    webView->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    // Enable focus for keyboard events
    setFocusPolicy(Qt::StrongFocus);
    
    // If we have a parent, fill it
    if (parent) {
        resize(parent->size());
        webView->resize(size());
    }
}

void Html::setHtml(const QString &html) {
    content = html;
    if (webView) {
        webView->setHtml(html);
    }
}

void Html::resizeEvent(QResizeEvent *event) {
    Element::resizeEvent(event);
    
    // Resize the web view to match the Html element size
    if (webView) {
        webView->resize(event->size());
    }
}

void Html::updateParentVisualState() {
    // Html elements have transparent background, similar to Text elements
    if (hasParent()) {
        // Keep transparent background
        setStyleSheet("QFrame { background-color: transparent; border: none; }");
    } else {
        // Keep transparent background  
        setStyleSheet("QFrame { background-color: transparent; border: none; }");
    }
    update(); // Trigger repaint
}