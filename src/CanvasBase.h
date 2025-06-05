#pragma once

#include <QWidget>
#include <QString>
#include <QList>

class Controls;
class Element;
class ClientRect;

class CanvasBase : public QWidget {
    Q_OBJECT
public:
    explicit CanvasBase(QWidget *parent = nullptr);
    virtual ~CanvasBase() = default;
    
    // Canvas state management
    QString getMode() const { return mode; }
    virtual void setMode(const QString &newMode);
    
    // Element creation
    virtual void createFrame();
    virtual void createText();
    virtual void createVariable();
    
    // Control management
    virtual void showControls(const QRect &rect);
    virtual void hideControls();
    
    // Selection management
    virtual void selectElement(const QString &elementId, bool addToSelection = false);
    virtual void updateControlsVisibility();
    
    // Rendering method to be overridden by subclasses
    virtual void render() = 0;
    
    // Get rendering type (CPU/GPU)
    virtual QString getRenderingType() const = 0;

signals:
    void modeChanged(const QString &newMode);
    void elementCreated(const QString &type, const QString &name);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    Controls *controls;
    
    // Canvas state
    QString mode;
    
    // Element management
    QList<Element*> elements;  // Stores all types of elements (Frame, Text, Variable)
    QList<QString> selectedElements;  // Stores IDs of selected elements
};