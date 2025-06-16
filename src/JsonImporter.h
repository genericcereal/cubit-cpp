#pragma once
#include <QObject>
#include <QString>
#include <QMap>

class Node;
class CreationManager;
class ElementModel;

class JsonImporter : public QObject {
    Q_OBJECT
    
public:
    explicit JsonImporter(QObject *parent = nullptr);
    ~JsonImporter() = default;
    
    void setCreationManager(CreationManager* creationManager) { m_creationManager = creationManager; }
    void setElementModel(ElementModel* elementModel) { m_elementModel = elementModel; }
    
    // Import operations
    QString createNodeFromJson(const QString &jsonData);
    QStringList createNodesFromJson(const QString &jsonData);
    void createGraphFromJson(const QString &jsonData);
    
signals:
    void nodeImported(const QString& nodeId);
    void nodesImported(const QStringList& nodeIds);
    void graphImported(int nodeCount, int edgeCount);
    void importError(const QString& error);
    
private:
    CreationManager* m_creationManager = nullptr;
    ElementModel* m_elementModel = nullptr;
    
    // Helper to migrate old port type names
    QString migratePortType(const QString& oldType);
};