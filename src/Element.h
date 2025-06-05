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
    
    // Canvas position management
    void setCanvasPosition(const QPoint &pos) { canvasPosition = pos; }
    QPoint getCanvasPosition() const { return canvasPosition; }
    void setCanvasSize(const QSize &size) { canvasSize = size; }
    QSize getCanvasSize() const { return canvasSize; }
    void updateVisualPosition(const QPoint &panOffset, qreal zoomScale = 1.0);
    
protected:
    ElementType elementType;
    int elementId;
    QPoint canvasPosition;  // Logical position on canvas (independent of pan)
    QSize canvasSize;  // Logical size on canvas (independent of zoom)
    
    // Override resize to track canvas size
    void resizeEvent(QResizeEvent *event) override;
};