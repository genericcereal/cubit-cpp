#pragma once
#include <QObject>
#include <QPointF>
#include <memory>
#include <unordered_map>
#include "Element.h"

class ElementModel;
class SelectionManager;
class CreationManager;
class HitTestService;
class JsonImporter;
class CommandHistory;
struct IModeHandler;

class CanvasController : public QObject {
    Q_OBJECT
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(CanvasType canvasType READ canvasType WRITE setCanvasType NOTIFY canvasTypeChanged)
    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(qreal savedContentX READ savedContentX WRITE setSavedContentX NOTIFY savedContentXChanged)
    Q_PROPERTY(qreal savedContentY READ savedContentY WRITE setSavedContentY NOTIFY savedContentYChanged)
    Q_PROPERTY(qreal savedZoom READ savedZoom WRITE setSavedZoom NOTIFY savedZoomChanged)
    
public:
    // Enum for canvas interaction modes
    enum class Mode {
        Select,
        Frame,
        Text
    };
    Q_ENUM(Mode)
    
    // Enum for canvas types
    enum class CanvasType {
        Design,
        Script,
        Variant
    };
    Q_ENUM(CanvasType)
    
public:
    explicit CanvasController(ElementModel& model,
                             SelectionManager& sel,
                             QObject *parent = nullptr);
    ~CanvasController();
    
    // Mode management
    Mode mode() const { return m_mode; }
    void setMode(Mode mode);
    
    // Canvas type management
    CanvasType canvasType() const { return m_canvasType; }
    void setCanvasType(CanvasType type);
    
    // Editing element management (for variant mode)
    void setEditingElement(QObject* editingElement);
    
    // Drag state (handled in QML now)
    bool isDragging() const { return false; }  // Always false since dragging is handled in QML
    
    
    // Helper methods (delegated to HitTestService)
    Q_INVOKABLE Element* hitTest(qreal x, qreal y);
    Q_INVOKABLE Element* hitTestForHover(qreal x, qreal y);
    
    // Undo/Redo support
    bool canUndo() const;
    bool canRedo() const;
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    
    // Viewport state management
    qreal savedContentX() const { return m_savedContentX; }
    qreal savedContentY() const { return m_savedContentY; }
    qreal savedZoom() const { return m_savedZoom; }
    void setSavedContentX(qreal x);
    void setSavedContentY(qreal y);
    void setSavedZoom(qreal zoom);
    
    
public slots:
    // Mouse handling
    void handleMousePress(qreal x, qreal y);
    void handleMouseMove(qreal x, qreal y);
    void handleMouseRelease(qreal x, qreal y);
    
    // Element creation (delegated to CreationManager)
    void createElement(const QString &type, qreal x, qreal y, qreal width = 200, qreal height = 150);
    Q_INVOKABLE void createVariable();
    Q_INVOKABLE void createNode(qreal x, qreal y, const QString &title = "Node", const QString &color = "");
    Q_INVOKABLE void createEdge(const QString &sourceNodeId, const QString &targetNodeId, 
                                const QString &sourceHandleType, const QString &targetHandleType,
                                int sourcePortIndex, int targetPortIndex);
    Q_INVOKABLE void createEdgeByPortId(const QString &sourceNodeId, const QString &targetNodeId,
                                        const QString &sourcePortId, const QString &targetPortId);
    
    // JSON-based creation (delegated to JsonImporter)
    Q_INVOKABLE QString createNodeFromJson(const QString &jsonData);
    Q_INVOKABLE void createNodesFromJson(const QString &jsonData);
    Q_INVOKABLE void createGraphFromJson(const QString &jsonData);
    
    // Component variant operations
    Q_INVOKABLE void duplicateVariant(const QString &variantId);
    
    // Selection
    void selectElementsInRect(const QRectF &rect);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void deleteSelectedElements();
    
signals:
    void modeChanged();
    void canvasTypeChanged();
    void isDraggingChanged();
    void elementCreated(Element *element);
    void canUndoChanged();
    void canRedoChanged();
    void savedContentXChanged();
    void savedContentYChanged();
    void savedZoomChanged();
    
protected:
    ElementModel& m_elementModel;
    SelectionManager& m_selectionManager;
    
private:
    Mode m_mode;
    CanvasType m_canvasType;
    
    // Subcontrollers
    std::unique_ptr<CreationManager> m_creationManager;
    std::unique_ptr<HitTestService> m_hitTestService;
    std::unique_ptr<JsonImporter> m_jsonImporter;
    std::unique_ptr<CommandHistory> m_commandHistory;
    
    // Mode handlers
    std::unordered_map<Mode, std::unique_ptr<IModeHandler>> m_modeHandlers;
    IModeHandler* m_currentHandler = nullptr;
    
    // Viewport state - initialize to invalid state so centerViewAtOrigin() is used on first load
    qreal m_savedContentX = 0.0;
    qreal m_savedContentY = 0.0;
    qreal m_savedZoom = 0.0;  // Invalid zoom triggers centerViewAtOrigin() in main.qml
    
    
    // Initialize subcontrollers
    void initializeModeHandlers();
    void updateSubcontrollersCanvasType();
};