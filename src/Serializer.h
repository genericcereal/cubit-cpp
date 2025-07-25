#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

class Project;
class Element;
class ElementModel;
class Application;
class Node;
class Edge;
class Scripts;

class Serializer : public QObject
{
    Q_OBJECT

public:
    explicit Serializer(Application* app, QObject *parent = nullptr);
    
    // Serialization methods
    QJsonObject serializeProject(Project* project) const;
    QJsonObject serializeElement(Element* element) const;
    
    // Deserialization methods
    Project* deserializeProject(const QJsonObject& projectData);
    Element* deserializeElement(const QJsonObject& elementData, ElementModel* model);
    
    // Script element serialization
    QJsonObject serializeNode(Node* node) const;
    QJsonObject serializeEdge(Edge* edge) const;
    Node* deserializeNode(const QJsonObject& nodeData, Scripts* scripts);
    Edge* deserializeEdge(const QJsonObject& edgeData, Scripts* scripts);
    
private:
    Application* m_application;
};

#endif // SERIALIZER_H