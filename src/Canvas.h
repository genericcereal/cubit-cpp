#pragma once

#include <QWidget>
#include <QString>
#include <QList>

class ActionsPanel;
class Controls;
class Element;
class ClientRect;
class FPSWidget;

class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);
    
    // Canvas state management
    QString getMode() const { return mode; }
    void setMode(const QString &newMode);
    
    // Element creation
    void createFrame();
    void createText();
    void createVariable();
    
    // Control management
    void showControls(const QRect &rect);
    void hideControls();
    
    // Selection management
    void selectElement(const QString &elementId, bool addToSelection = false);
    void updateControlsVisibility();

signals:
    void modeChanged(const QString &newMode);
    void elementCreated(const QString &type, const QString &name);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    ActionsPanel *actionsPanel;
    Controls *controls;
    FPSWidget *fpsWidget;
    
    // Canvas state
    QString mode;
    
    // Element management
    QList<Element*> elements;  // Stores all types of elements (Frame, Text, Variable)
    QList<QString> selectedElements;  // Stores IDs of selected elements
    
    // Frame creation state
};