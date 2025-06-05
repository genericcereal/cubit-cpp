#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QString>
#include <QList>

class Controls;
class Element;
class ClientRect;

class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GLCanvas(QWidget *parent = nullptr);
    ~GLCanvas();
    
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
    
    // Rendering
    void render();
    
    // Get rendering type
    QString getRenderingType() const { return "GPU"; }
    
    // Static method to check if OpenGL is available
    static bool isOpenGLAvailable();

protected:
    // OpenGL functions
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    // Override paint event to trigger OpenGL rendering
    void paintEvent(QPaintEvent *event) override;
    
    // Mouse events
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void modeChanged(const QString &newMode);
    void elementCreated(const QString &type, const QString &name);
    
private:
    // Canvas members
    Controls *controls;
    QString mode;
    QList<Element*> elements;
    QList<QString> selectedElements;
    // Shader programs
    QOpenGLShaderProgram *shaderProgram;
    
    // Projection matrix
    QMatrix4x4 projectionMatrix;
    
    // OpenGL objects
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    
    // Helper methods
    void initShaders();
    void renderFrame(const QRect &rect, const QColor &color);
    void renderText(const QString &text, const QPoint &pos, const QFont &font);
    
    bool initialized;
};