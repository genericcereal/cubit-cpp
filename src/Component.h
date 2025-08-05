#ifndef COMPONENTELEMENT_H
#define COMPONENTELEMENT_H

#include "Element.h"
#include <QString>
#include <QList>

class ComponentElement : public Element
{
    Q_OBJECT
    Q_PROPERTY(QList<Element*> elements READ elements NOTIFY elementsChanged)

public:
    explicit ComponentElement(const QString &id, QObject *parent = nullptr);
    virtual ~ComponentElement();

    // Elements management
    QList<Element*> elements() const { return m_elements; }
    void addElement(Element* element);
    void removeElement(Element* element);
    
    // For deserialization - store element IDs until they can be resolved
    void setPendingElementIds(const QStringList& ids) { m_pendingElementIds = ids; }
    QStringList pendingElementIds() const { return m_pendingElementIds; }

signals:
    void elementsChanged();
    void elementAdded(Element* element);
    void elementRemoved(Element* element);

private:
    QList<Element*> m_elements;
    QStringList m_pendingElementIds;  // Used during deserialization
    
    Q_DISABLE_COPY(ComponentElement)
};

#endif // COMPONENTELEMENT_H