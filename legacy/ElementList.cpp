#include "ElementList.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QString>

ElementList::ElementList(QWidget *parent) : QWidget(parent), frameCount(0) {
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    
    headerLabel = new QLabel("Elements", this);
    headerLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    layout->addWidget(headerLabel);
    
    layout->addStretch();
}

void ElementList::addElement(const QString &type, const QString &name) {
    QLabel *elementLabel = new QLabel(name, this);
    
    // Style based on element type
    if (type == "Frame") {
        elementLabel->setStyleSheet("padding: 3px 10px; color: #333;");
    } else if (type == "Text") {
        elementLabel->setStyleSheet("padding: 3px 10px; color: #666;");
    } else if (type == "Variable") {
        elementLabel->setStyleSheet("padding: 3px 10px; color: #4a90e2; font-style: italic;");
    }
    
    // Insert before the stretch
    layout->insertWidget(layout->count() - 1, elementLabel);
    frameLabels.append(elementLabel);
}

void ElementList::clear() {
    for (QLabel *label : frameLabels) {
        layout->removeWidget(label);
        delete label;
    }
    frameLabels.clear();
    frameCount = 0;
}