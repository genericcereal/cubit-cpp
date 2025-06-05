#pragma once

#include <QWidget>
#include <QString>
#include <QList>

class PanelLayer;
class ControlLayer;
class ElementLayer;
class ClientRectLayer;
class Element;

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

signals:
    void modeChanged(const QString &newMode);
    void elementCreated(const QString &type, const QString &name);

protected:
    void resizeEvent(QResizeEvent *event) override;
    // void mousePressEvent(QMouseEvent *event) override;
    // void mouseMoveEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;

private:
    PanelLayer *panelLayer;
    ControlLayer *controlLayer;
    ElementLayer *elementLayer;
    ClientRectLayer *clientRectLayer;
    
    // Canvas state
    QString mode;
    
    // Element management
    QList<Element*> elements;  // Stores all types of elements (Frame, Text, Variable)
    
    // Frame creation state
    bool isCreatingFrame;
    QPoint frameStartPos;
    class Frame *tempFrame;
    class ClientRect *tempClientRect;
};