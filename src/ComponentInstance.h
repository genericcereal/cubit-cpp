#ifndef COMPONENTINSTANCE_H
#define COMPONENTINSTANCE_H

#include <QString>

class ComponentVariant;
class Frame;
class ElementModel;
class Element;

class ComponentInstance
{
public:
    explicit ComponentInstance(const QString& elementId);
    virtual ~ComponentInstance() = default;

    QString elementId() const;
    ComponentVariant* masterVariant() const;
    void setMasterVariant(ComponentVariant* variant);
    
    // Global instance management
    bool isGlobalInstance() const { return m_isGlobalInstance; }
    void setIsGlobalInstance(bool isInstance) { m_isGlobalInstance = isInstance; }
    
    // Global frame instance management methods
    void handleGlobalFrameParenting(const QString& oldParentId, const QString& newParentId, Element* element);
    bool isGlobalFrame(Element* frame) const;
    void createInstancesInAllFrames(Element* element);
    void removeInstancesFromAllFrames(Element* element);
    void createInstanceInFrame(Frame* targetFrame, ElementModel* targetModel, Element* element);

protected:
    QString m_elementId;
    ComponentVariant* m_masterVariant = nullptr;
    bool m_isGlobalInstance = false;
};

#endif // COMPONENTINSTANCE_H