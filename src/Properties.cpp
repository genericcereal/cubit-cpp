#include "Properties.h"
#include "Canvas.h"
#include "Frame.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>

// Property class implementation
class Property : public QWidget {
public:
    Property(const QString &labelText, int inputCount = 1, QWidget *parent = nullptr) 
        : QWidget(parent) {
        // Create horizontal layout with no margins
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 5, 0, 5);
        layout->setSpacing(10);
        
        // Create label (left section)
        QLabel *label = new QLabel(labelText, this);
        label->setMinimumWidth(100);
        label->setMaximumWidth(100);
        label->setStyleSheet("QLabel { color: #666; }");
        layout->addWidget(label);
        
        // Create input area (right section)
        QWidget *inputArea = new QWidget(this);
        QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
        inputLayout->setContentsMargins(0, 0, 0, 0);
        inputLayout->setSpacing(5);
        
        // Add input(s)
        for (int i = 0; i < inputCount; ++i) {
            QLineEdit *input = new QLineEdit(this);
            input->setStyleSheet("QLineEdit { background-color: #f5f5f5; border: 1px solid #ddd; padding: 4px; border-radius: 3px; }");
            input->setReadOnly(true);  // Make read-only for now
            inputLayout->addWidget(input);
            inputs.append(input);
        }
        
        layout->addWidget(inputArea);
        
        // Set size policy to fill width
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    
    // Get input value(s)
    QString getValue(int index = 0) const {
        if (index >= 0 && index < inputs.size()) {
            return inputs[index]->text();
        }
        return QString();
    }
    
    // Set input value(s)
    void setValue(const QString &value, int index = 0) {
        if (index >= 0 && index < inputs.size()) {
            inputs[index]->setText(value);
        }
    }
    
private:
    QList<QLineEdit*> inputs;
};

Properties::Properties(QWidget *parent) : QWidget(parent), canvasRef(nullptr) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(0);
    
    // Add a header
    QLabel *header = new QLabel("Properties", this);
    header->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; margin-bottom: 10px; }");
    layout->addWidget(header);
    
    // Add properties
    nameProperty = new Property("Name", 1, this);
    layout->addWidget(nameProperty);
    
    positionProperty = new Property("Position", 2, this);
    layout->addWidget(positionProperty);
    
    sizeProperty = new Property("Size", 2, this);
    layout->addWidget(sizeProperty);
    
    rotationProperty = new Property("Rotation", 1, this);
    rotationProperty->setValue("0°");
    layout->addWidget(rotationProperty);
    
    overflowProperty = new Property("Overflow", 1, this);
    overflowProperty->setValue("hidden");
    layout->addWidget(overflowProperty);
    
    // Add stretch to push everything to the top
    layout->addStretch();
}

void Properties::setCanvas(Canvas *canvas) {
    canvasRef = canvas;
    
    // Connect to canvas signals
    if (canvasRef) {
        connect(canvasRef, &Canvas::selectionChanged, this, &Properties::updateFromSelection);
        connect(canvasRef, &Canvas::propertiesChanged, this, &Properties::updateFromSelection);
        // Update immediately with current selection
        updateFromSelection();
    }
}

void Properties::updateFromSelection() {
    if (!canvasRef) return;
    
    // Get selected frames
    QList<Frame*> selectedFrames = canvasRef->getSelectedFrames();
    
    if (selectedFrames.isEmpty()) {
        // Clear all properties when nothing is selected
        nameProperty->setValue("");
        positionProperty->setValue("", 0);
        positionProperty->setValue("", 1);
        sizeProperty->setValue("", 0);
        sizeProperty->setValue("", 1);
        rotationProperty->setValue("");
        overflowProperty->setValue("");
        return;
    }
    
    // For now, just show the first selected frame's properties
    Frame *frame = selectedFrames.first();
    
    // Update name
    nameProperty->setValue(frame->objectName());
    
    // Update position
    QPoint canvasPos = frame->getCanvasPosition();
    positionProperty->setValue(QString::number(canvasPos.x()), 0);
    positionProperty->setValue(QString::number(canvasPos.y()), 1);
    
    // Update size
    QSize canvasSize = frame->getCanvasSize();
    sizeProperty->setValue(QString::number(canvasSize.width()), 0);
    sizeProperty->setValue(QString::number(canvasSize.height()), 1);
    
    // Update rotation (for now, always 0)
    rotationProperty->setValue("0°");
    
    // Update overflow
    overflowProperty->setValue(frame->getOverflow());
}