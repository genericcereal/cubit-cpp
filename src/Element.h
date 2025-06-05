#pragma once
#include <QFrame>
#include <QString>

// Base class for all canvas elements (Frame, Text, Variable, etc.)
class Element : public QFrame {
    Q_OBJECT
public:
    enum ElementType {
        FrameType,
        TextType,
        VariableType
    };

    explicit Element(ElementType type, int id, QWidget *parent = nullptr);
    virtual ~Element() = default;
    
    // Pure virtual methods that all elements must implement
    virtual QString getName() const = 0;
    virtual int getId() const = 0;
    
    // Common methods
    ElementType getType() const { return elementType; }
    QString getTypeName() const;
    
protected:
    ElementType elementType;
    int elementId;
};