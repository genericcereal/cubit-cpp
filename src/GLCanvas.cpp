#include "GLCanvas.h"
#include "Frame.h"
#include "Text.h"
#include "Variable.h"
#include "Element.h"
#include "Controls.h"
#include "ClientRect.h"
#include <QOpenGLContext>
#include <QPainter>
#include <QMouseEvent>

GLCanvas::GLCanvas(QWidget *parent) 
    : QOpenGLWidget(parent), shaderProgram(nullptr), initialized(false), mode("Select") {
    // Enable OpenGL
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    setFormat(format);
    
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);  // Enable mouse tracking for cursor changes
    
    // Create UI controls directly on Canvas
    controls = new Controls(this);
    controls->hide();  // Initially hidden
    
    // Ensure controls are on top
    controls->raise();
}

GLCanvas::~GLCanvas() {
    makeCurrent();
    if (vao.isCreated()) {
        vao.destroy();
    }
    if (vbo.isCreated()) {
        vbo.destroy();
    }
    delete shaderProgram;
    doneCurrent();
}

bool GLCanvas::isOpenGLAvailable() {
    QOpenGLContext testContext;
    testContext.create();
    return testContext.isValid();
}

void GLCanvas::initializeGL() {
    initializeOpenGLFunctions();
    
    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    
    // Initialize shaders
    initShaders();
    
    // Setup VAO and VBO
    if (!vao.isCreated()) {
        vao.create();
    }
    if (!vbo.isCreated()) {
        vbo.create();
    }
    
    initialized = true;
}

void GLCanvas::initShaders() {
    shaderProgram = new QOpenGLShaderProgram();
    
    // Vertex shader
    const char *vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 projection;
        uniform mat4 model;
        void main() {
            gl_Position = projection * model * vec4(aPos, 1.0);
        }
    )";
    
    // Fragment shader
    const char *fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 color;
        void main() {
            FragColor = color;
        }
    )";
    
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shaderProgram->link();
}

void GLCanvas::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(0, w, h, 0, -1, 1);  // Top-left origin like Qt
}

void GLCanvas::paintGL() {
    if (!initialized) return;
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render all elements
    for (Element *element : elements) {
        if (element->getType() == Element::FrameType) {
            Frame *frame = qobject_cast<Frame*>(element);
            if (frame) {
                renderFrame(frame->geometry(), frame->getColor());
            }
        }
    }
    
    // For text rendering, we'll use QPainter overlay
    // This allows us to leverage Qt's text rendering while using OpenGL for shapes
}

void GLCanvas::paintEvent(QPaintEvent *event) {
    // Call OpenGL painting first
    QOpenGLWidget::paintEvent(event);
    
    // Now use QPainter for text and other complex elements
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Render text elements
    for (Element *element : elements) {
        if (element->getType() == Element::TextType) {
            Text *text = qobject_cast<Text*>(element);
            if (text) {
                painter.setFont(text->font());
                painter.setPen(text->palette().color(QPalette::WindowText));
                painter.drawText(text->geometry(), Qt::AlignLeft | Qt::AlignTop, text->getText());
            }
        }
    }
}

void GLCanvas::render() {
    QOpenGLWidget::update();  // Triggers paintGL
}

void GLCanvas::renderFrame(const QRect &rect, const QColor &color) {
    if (!initialized || !shaderProgram) return;
    
    shaderProgram->bind();
    
    // Set projection matrix
    shaderProgram->setUniformValue("projection", projectionMatrix);
    
    // Set model matrix (translation and scale)
    QMatrix4x4 model;
    model.translate(rect.x(), rect.y(), 0);
    shaderProgram->setUniformValue("model", model);
    
    // Set color
    shaderProgram->setUniformValue("color", color);
    
    // Define vertices for a rectangle
    float vertices[] = {
        0.0f, 0.0f, 0.0f,                    // Top-left
        rect.width(), 0.0f, 0.0f,            // Top-right
        rect.width(), rect.height(), 0.0f,   // Bottom-right
        0.0f, rect.height(), 0.0f            // Bottom-left
    };
    
    // Bind VAO and VBO
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));
    
    // Set vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    
    // Draw rectangle
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    vbo.release();
    shaderProgram->release();
}

void GLCanvas::renderText(const QString &, const QPoint &, const QFont &) {
    // Text rendering is handled in paintEvent using QPainter
    // This method is kept for future pure OpenGL text rendering if needed
}

// Canvas implementation
void GLCanvas::setMode(const QString &newMode) {
    if (mode != newMode) {
        mode = newMode;
        emit modeChanged(mode);
        
        // Handle mode-specific actions
        if (newMode == "Text") {
            createText();
        } else if (newMode == "Variable") {
            createVariable();
        }
    }
}

void GLCanvas::showControls(const QRect &rect) {
    if (controls) {
        controls->updateGeometry(rect);
        controls->show();
        controls->raise();
    }
}

void GLCanvas::hideControls() {
    if (controls) {
        controls->hide();
    }
}

void GLCanvas::selectElement(const QString &elementId, bool addToSelection) {
    if (!addToSelection) {
        selectedElements.clear();
    }
    
    if (!selectedElements.contains(elementId)) {
        selectedElements.append(elementId);
    }
    
    updateControlsVisibility();
}

void GLCanvas::updateControlsVisibility() {
    if (selectedElements.isEmpty()) {
        hideControls();
        return;
    }
    
    bool hasFrame = false;
    QRect boundingRect;
    bool firstFrame = true;
    
    for (const QString &id : selectedElements) {
        for (Element *element : elements) {
            if (QString::number(element->getId()) == id) {
                if (element->getType() == Element::FrameType) {
                    hasFrame = true;
                    if (firstFrame) {
                        boundingRect = element->geometry();
                        firstFrame = false;
                    } else {
                        boundingRect = boundingRect.united(element->geometry());
                    }
                }
                break;
            }
        }
    }
    
    if (hasFrame) {
        showControls(boundingRect);
    } else {
        hideControls();
    }
}

void GLCanvas::createFrame() {
    // Frame creation happens via mouse events
}

void GLCanvas::createText() {
    int textId = elements.size() + 1;
    Text *text = new Text(textId, this);
    
    int textX = (width() - text->width()) / 2;
    int textY = (height() - text->height()) / 2;
    text->move(textX, textY);
    text->hide();  // Don't show as widget in OpenGL mode
    
    elements.append(text);
    emit elementCreated("Text", text->getName());
    setMode("Select");
    
    // Trigger OpenGL repaint
    update();
}

void GLCanvas::createVariable() {
    int variableId = elements.size() + 1;
    Variable *variable = new Variable(variableId, nullptr);
    
    elements.append(variable);
    emit elementCreated("Variable", variable->getName());
    setMode("Select");
}

void GLCanvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (mode == "Frame") {
            int frameId = elements.size() + 1;
            
            Frame *frame = new Frame(frameId, this);
            frame->resize(400, 400);
            frame->move(event->pos());
            frame->hide();  // Don't show as widget in OpenGL mode
            
            // For now, skip ClientRect in OpenGL mode since it would need special handling
            // TODO: Implement ClientRect rendering in OpenGL
            
            elements.append(frame);
            
            selectedElements.clear();
            selectedElements.append(QString::number(frame->getId()));
            updateControlsVisibility();
            
            if (controls) {
                controls->raise();
            }
            
            emit elementCreated("Frame", frame->getName());
            setMode("Select");
            
            // Trigger OpenGL repaint
            update();
            
        } else {
            QWidget *widget = childAt(event->pos());
            
            if (!widget || widget == this) {
                selectedElements.clear();
                updateControlsVisibility();
            }
        }
    }
    
    QOpenGLWidget::mousePressEvent(event);
}

void GLCanvas::mouseMoveEvent(QMouseEvent *event) {
    QOpenGLWidget::mouseMoveEvent(event);
}

void GLCanvas::mouseReleaseEvent(QMouseEvent *event) {
    QOpenGLWidget::mouseReleaseEvent(event);
}