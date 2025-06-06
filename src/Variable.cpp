#include "Variable.h"
#include <QLabel>
#include <QVBoxLayout>

Variable::Variable(int id, QWidget *parent) : Element(ElementType::VariableType, id, parent) {
    variableName = QString("Variable_%1").arg(elementId, 2, 10, QChar('0'));
    setFixedSize(150, 80);
    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);
    setStyleSheet("");
    
    // Create labels to display variable info
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(5, 5, 5, 5);
    
    QLabel *nameLabel = new QLabel(variableName, this);
    nameLabel->setStyleSheet("font-weight: bold; color: #2c5aa0;");
    nameLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *valueLabel = new QLabel("undefined", this);
    valueLabel->setObjectName("valueLabel");
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("color: #666;");
    
    layout->addWidget(nameLabel);
    layout->addWidget(valueLabel);
    
    // Set object name for debugging/identification
    setObjectName(variableName);
    
    // Initialize with undefined value
    value = QVariant();
}

void Variable::setValue(const QVariant &newValue) {
    value = newValue;
    
    // Update the value label
    QLabel *valueLabel = findChild<QLabel*>("valueLabel");
    if (valueLabel) {
        if (value.isValid()) {
            valueLabel->setText(value.toString());
        } else {
            valueLabel->setText("undefined");
        }
    }
}

QString Variable::getType() const {
    if (!value.isValid()) {
        return "undefined";
    }
    
    switch (value.type()) {
        case QVariant::Int:
        case QVariant::LongLong:
            return "Number";
        case QVariant::Double:
            return "Number";
        case QVariant::String:
            return "String";
        case QVariant::Bool:
            return "Boolean";
        default:
            return "Unknown";
    }
}

void Variable::updateParentVisualState() {
    // Variables don't visually show on the canvas, so no visual update needed
    // But we still implement the pure virtual function
}