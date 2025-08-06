#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include "CanvasController.h"
#include "SelectionManager.h"
#include "ElementModel.h"
#include "Scripts.h"
#include "DesignElement.h"
#include "ScriptExecutor.h"
#include "PrototypeController.h"
#include "PlatformConfig.h"

class CanvasContext;
class StreamingAIClient;
class AuthenticationManager;
class ConsoleMessageRepository;
class VariableBindingManager;
Q_DECLARE_OPAQUE_POINTER(ConsoleMessageRepository*)

// class Component; // Component system removed
// Q_DECLARE_OPAQUE_POINTER(Component*)
Q_DECLARE_OPAQUE_POINTER(VariableBindingManager*)

Q_DECLARE_OPAQUE_POINTER(CanvasContext*)

class Project : public QObject {
    Q_OBJECT
    Q_PROPERTY(CanvasController* controller READ controller CONSTANT)
    Q_PROPERTY(SelectionManager* selectionManager READ selectionManager CONSTANT)
    Q_PROPERTY(ElementModel* elementModel READ elementModel CONSTANT)
    Q_PROPERTY(Scripts* scripts READ scripts CONSTANT)
    Q_PROPERTY(PrototypeController* prototypeController READ prototypeController CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(CanvasContext* currentContext READ currentContext NOTIFY currentContextChanged)
    Q_PROPERTY(QObject* editingElement READ editingElement NOTIFY editingElementChanged)
    Q_PROPERTY(Scripts* activeScripts READ activeScripts NOTIFY activeScriptsChanged)
    Q_PROPERTY(QQmlListProperty<PlatformConfig> platforms READ platforms NOTIFY platformsChanged)
    Q_PROPERTY(ConsoleMessageRepository* console READ console CONSTANT)
    Q_PROPERTY(QString draggedVariableType READ draggedVariableType WRITE setDraggedVariableType NOTIFY draggedVariableTypeChanged)
    Q_PROPERTY(VariableBindingManager* bindingManager READ bindingManager CONSTANT)
    Q_PROPERTY(QVariantMap hoveredVariableTarget READ hoveredVariableTarget WRITE setHoveredVariableTarget NOTIFY hoveredVariableTargetChanged)

public:
    explicit Project(const QString& id, const QString& name = QString(), QObject *parent = nullptr);
    ~Project();

    // Property getters
    CanvasController* controller() const;
    SelectionManager* selectionManager() const;
    ElementModel* elementModel() const;
    Scripts* scripts() const;
    PrototypeController* prototypeController() const;
    QString name() const;
    QString id() const;
    QString viewMode() const;
    CanvasContext* currentContext() const;
    QObject* editingElement() const;
    Scripts* activeScripts() const;
    QQmlListProperty<PlatformConfig> platforms();
    ConsoleMessageRepository* console() const;
    QString draggedVariableType() const;
    VariableBindingManager* bindingManager() const;
    QVariantMap hoveredVariableTarget() const;
    
    // Platform management
    Q_INVOKABLE void addPlatform(const QString& platformName);
    Q_INVOKABLE void removePlatform(const QString& platformName);
    Q_INVOKABLE PlatformConfig* getPlatform(const QString& platformName) const;
    Q_INVOKABLE QList<PlatformConfig*> getAllPlatforms() const;
    Q_INVOKABLE Scripts* getPlatformScripts(const QString& platformName) const;
    void addPlatformDirectly(const QString& platformName);  // Direct method for command to use

    // Property setters
    void setName(const QString& name);
    void setViewMode(const QString& viewMode);
    void setCanvasContext(std::unique_ptr<CanvasContext> context);
    void setId(const QString& id);  // Update project ID (e.g., after API sync)
    Q_INVOKABLE void setEditingElement(QObject* element, const QString& viewMode = QString());
    Q_INVOKABLE void setEditingPlatform(PlatformConfig* platform, const QString& viewMode = QString());
    void setDraggedVariableType(const QString& type);
    void setHoveredVariableTarget(const QVariantMap& target);

    // Initialize the canvas components
    void initialize();
    
    // Execute a script event
    Q_INVOKABLE void executeScriptEvent(const QString& eventName);
    
    // AI command handling
    Q_INVOKABLE void handleAICommand(const QString& prompt);
    Q_INVOKABLE void handleAIContinuationResponse(bool accepted, const QString& feedback);
    
private slots:
    void onAICommandReceived(const QString& prompt, QObject* project);
    void onAIContinuationResponse(bool accepted, const QString& feedback, QObject* project);
    void onAIModeDisabled();

signals:
    void nameChanged();
    void viewModeChanged();
    void currentContextChanged();
    void editingElementChanged();
    void activeScriptsChanged();
    void platformsChanged();
    void viewportStateShouldBeSaved();
    void viewportStateShouldBeRestored();
    void draggedVariableTypeChanged();
    void hoveredVariableTargetChanged();

private:
    std::unique_ptr<CanvasController> m_controller;
    std::unique_ptr<SelectionManager> m_selectionManager;
    std::unique_ptr<ElementModel> m_elementModel;
    std::unique_ptr<Scripts> m_scripts;
    std::unique_ptr<ScriptExecutor> m_scriptExecutor;
    std::unique_ptr<PrototypeController> m_prototypeController;
    std::unique_ptr<StreamingAIClient> m_aiClient;
    std::unique_ptr<ConsoleMessageRepository> m_console;
    std::unique_ptr<VariableBindingManager> m_bindingManager;
    QString m_name;
    QString m_id;
    QString m_viewMode;
    std::unique_ptr<CanvasContext> m_currentContext;
    QObject* m_editingElement = nullptr; // Can be DesignElement* or Component*
    std::vector<std::unique_ptr<PlatformConfig>> m_platforms;
    QString m_draggedVariableType;
    QVariantMap m_hoveredVariableTarget;
    
    // QML list property helpers
    static void appendPlatform(QQmlListProperty<PlatformConfig>* list, PlatformConfig* platform);
    static qsizetype platformCount(QQmlListProperty<PlatformConfig>* list);
    static PlatformConfig* platformAt(QQmlListProperty<PlatformConfig>* list, qsizetype index);
    static void clearPlatforms(QQmlListProperty<PlatformConfig>* list);
    
    void updateActiveScripts();
    void createContextForViewMode(const QString& mode);
};

#endif // PROJECT_H