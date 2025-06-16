#pragma once
#include <QObject>
#include <QPointF>
#include <memory>
#include "Element.h"

class ElementModel;
class SelectionManager;
class DragManager;
class CreationManager;
class HitTestService;
class JsonImporter;
class CommandHistory;

class CanvasController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString canvasType READ canvasType WRITE setCanvasType NOTIFY canvasTypeChanged)
    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    
public:
    // Enum for canvas interaction modes
    enum class Mode {
        Select,
        Frame,
        Text,
        Html,
        Variable
    };
    Q_ENUM(Mode)
    
    // Enum for canvas types
    enum class CanvasType {
        Design,
        Script
    };
    Q_ENUM(CanvasType)
    
public:
    explicit CanvasController(QObject *parent = nullptr);
    ~CanvasController();
    
    // Mode management
    QString mode() const { return modeToString(m_mode); }
    Q_INVOKABLE void setMode(const QString &mode);
    
    // Canvas type management
    QString canvasType() const { return canvasTypeToString(m_canvasType); }
    Q_INVOKABLE void setCanvasType(const QString &type);
    
    // Drag state (delegated to DragManager)
    bool isDragging() const;
    
    // Element model and selection manager
    Q_INVOKABLE void setElementModel(ElementModel *model);
    Q_INVOKABLE void setSelectionManager(SelectionManager *manager);
    
    // Helper methods (delegated to HitTestService)
    Q_INVOKABLE Element* hitTest(qreal x, qreal y);
    
    // Undo/Redo support
    bool canUndo() const;
    bool canRedo() const;
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    
    
public slots:
    // Mouse handling
    void handleMousePress(qreal x, qreal y);
    void handleMouseMove(qreal x, qreal y);
    void handleMouseRelease(qreal x, qreal y);
    
    // Element creation (delegated to CreationManager)
    void createElement(const QString &type, qreal x, qreal y, qreal width = 200, qreal height = 150);
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
    
    // Selection
    void selectElementsInRect(const QRectF &rect);
    void selectAll();
    void deleteSelectedElements();
    
signals:
    void modeChanged();
    void canvasTypeChanged();
    void isDraggingChanged();
    void elementCreated(Element *element);
    void canUndoChanged();
    void canRedoChanged();
    
private:
    Mode m_mode;
    CanvasType m_canvasType;
    ElementModel *m_elementModel;
    SelectionManager *m_selectionManager;
    
    // Subcontrollers
    std::unique_ptr<DragManager> m_dragManager;
    std::unique_ptr<CreationManager> m_creationManager;
    std::unique_ptr<HitTestService> m_hitTestService;
    std::unique_ptr<JsonImporter> m_jsonImporter;
    std::unique_ptr<CommandHistory> m_commandHistory;
    
    // Helper methods for enum conversion
    static Mode modeFromString(const QString &str);
    static QString modeToString(Mode mode);
    static CanvasType canvasTypeFromString(const QString &str);
    static QString canvasTypeToString(CanvasType type);
    
    // Creation state for drag-to-create
    QPointF m_creationStartPos;
    Element* m_creationElement = nullptr;
    
    // Initialize subcontrollers
    void initializeSubcontrollers();
    void updateSubcontrollersCanvasType();
};