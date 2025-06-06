#include "Text.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QEvent>

Text::Text(int id, QWidget *parent) : Element(ElementType::TextType, id, parent), textId(id), editing(false), editWidget(nullptr) {
    textName = QString("Text_%1").arg(textId, 2, 10, QChar('0'));
    setFixedSize(200, 100);
    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("QFrame { background-color: transparent; border: none; }");
    
    // Set object name for debugging/identification
    setObjectName(textName);
    
    content = "Text Element";
    
    // IMPORTANT: Text elements should not handle mouse events directly
    // Mouse events are handled by the associated ClientRect
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    // Enable focus for keyboard events
    setFocusPolicy(Qt::StrongFocus);
}

void Text::setText(const QString &text) {
    content = text;
    update();  // Trigger repaint
}

void Text::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);  // Draw the frame if needed
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Set text color
    painter.setPen(Qt::black);
    
    // Get the rectangle where we can draw text
    QRect textRect = rect();
    
    // Calculate elided text that fits in the available width
    QFontMetrics metrics(painter.font());
    QString elidedText = metrics.elidedText(content, Qt::ElideRight, textRect.width());
    
    // Draw the text at top-left
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, elidedText);
}

void Text::startEditing() {
    if (editing) return;
    
    editing = true;
    
    // Temporarily disable transparent mouse events to allow editing
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    
    // Create line edit widget
    editWidget = new QLineEdit(this);
    editWidget->setText(content);
    editWidget->setFrame(false);
    editWidget->setStyleSheet("QLineEdit { background-color: white; padding: 0px; margin: 0px; }");
    
    // Position and size the edit widget
    editWidget->move(0, 0);
    editWidget->resize(size());
    editWidget->show();
    editWidget->setFocus();
    editWidget->selectAll();
    
    // Connect to handle Enter/Return key
    connect(editWidget, &QLineEdit::returnPressed, [this]() {
        endEditing(true);
    });
    
    // Install event filter to handle focus loss
    editWidget->installEventFilter(this);
    
    update();
}

void Text::endEditing(bool save) {
    if (!editing || !editWidget) return;
    
    if (save) {
        content = editWidget->text();
    }
    
    // Clean up edit widget
    editWidget->deleteLater();
    editWidget = nullptr;
    editing = false;
    
    // Re-enable transparent mouse events
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    update();
}

void Text::keyPressEvent(QKeyEvent *event) {
    if (editing) {
        if (event->key() == Qt::Key_Escape) {
            endEditing(false);  // Cancel editing
            event->accept();
            return;
        }
    }
    QFrame::keyPressEvent(event);
}

void Text::focusOutEvent(QFocusEvent *event) {
    // Don't end editing here - we'll handle it in eventFilter
    QFrame::focusOutEvent(event);
}

bool Text::eventFilter(QObject *watched, QEvent *event) {
    if (watched == editWidget && event->type() == QEvent::FocusOut) {
        // Check if the focus is moving to a child widget or staying within our text editor
        QFocusEvent *focusEvent = static_cast<QFocusEvent*>(event);
        if (focusEvent->reason() != Qt::PopupFocusReason) {
            // End editing when focus leaves the edit widget
            endEditing(true);
        }
    }
    return QFrame::eventFilter(watched, event);
}