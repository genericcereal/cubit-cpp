#include "Text.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QTextEdit>
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
    
    // Set font size to 19
    QFont font = painter.font();
    font.setPointSize(19);
    painter.setFont(font);
    
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
    
    // Create text edit widget
    editWidget = new QTextEdit(this);
    editWidget->setPlainText(content);
    editWidget->setFrameStyle(QFrame::NoFrame);
    
    // Set font size to 19
    QFont font = editWidget->font();
    font.setPointSize(19);
    editWidget->setFont(font);
    
    // Configure for single-line behavior
    editWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    editWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    editWidget->setWordWrapMode(QTextOption::NoWrap);
    editWidget->setAcceptRichText(false);
    
    // Set document margins to 0 for proper alignment
    editWidget->document()->setDocumentMargin(0);
    
    editWidget->setStyleSheet("QTextEdit { background-color: white; padding: 0px; margin: 0px; }");
    
    // Position and size the edit widget
    editWidget->move(0, 0);
    editWidget->resize(size());
    editWidget->show();
    editWidget->setFocus();
    editWidget->selectAll();
    
    // Install event filter to handle focus loss
    editWidget->installEventFilter(this);
    
    update();
}

void Text::endEditing(bool save) {
    if (!editing || !editWidget) return;
    
    if (save) {
        content = editWidget->toPlainText();
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
    if (editing && editWidget) {
        if (event->key() == Qt::Key_Escape) {
            endEditing(false);  // Cancel editing
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            endEditing(true);  // Save on Enter
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
    if (watched == editWidget) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                endEditing(true);
                return true;  // Consume the event
            } else if (keyEvent->key() == Qt::Key_Escape) {
                endEditing(false);
                return true;  // Consume the event
            }
        } else if (event->type() == QEvent::FocusOut) {
            // Check if the focus is moving to a child widget or staying within our text editor
            QFocusEvent *focusEvent = static_cast<QFocusEvent*>(event);
            if (focusEvent->reason() != Qt::PopupFocusReason) {
                // End editing when focus leaves the edit widget
                endEditing(true);
            }
        }
    }
    return QFrame::eventFilter(watched, event);
}