#include "Text.h"
#include <QLabel>
#include <QVBoxLayout>

Text::Text(int id, QWidget *parent) : Element(ElementType::TextType, id, parent), textId(id) {
    textName = QString("Text_%1").arg(textId, 2, 10, QChar('0'));
    setFixedSize(200, 100);
    setFrameStyle(QFrame::Box);
    setLineWidth(1);
    setStyleSheet("QFrame { background-color: #f5f5f5; border: 1px solid #999; }");
    
    // Create a label to display text content
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Text Element", this);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    layout->addWidget(label);
    
    // Set object name for debugging/identification
    setObjectName(textName);
    
    content = "Text Element";
    
    // IMPORTANT: Text elements should not handle mouse events directly
    // Mouse events are handled by the associated ClientRect
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void Text::setText(const QString &text) {
    content = text;
    // Update the label when we have one
    QLabel *label = findChild<QLabel*>();
    if (label) {
        label->setText(content);
    }
}