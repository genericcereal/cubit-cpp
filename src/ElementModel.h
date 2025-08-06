#pragma once
#include <QAbstractListModel>
#include <QList>
#include "Element.h"

class ElementModel : public QAbstractListModel {
    Q_OBJECT
    
public:
    enum ElementRoles {
        ElementRole = Qt::UserRole + 1,
        ElementIdRole,
        ElementTypeRole,
        NameRole,
        XRole,
        YRole,
        WidthRole,
        HeightRole,
        ParentIdRole,
        SelectedRole
    };
    
    explicit ElementModel(QObject *parent = nullptr);
    ~ElementModel();
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // Element management
    Q_INVOKABLE void addElement(Element *element);
    Q_INVOKABLE void removeElement(const QString &elementId);
    Q_INVOKABLE void removeElement(Element *element);
    Q_INVOKABLE void removeElementWithoutDelete(Element *element);
    Q_INVOKABLE Element* getElementById(const QString &elementId) const;
    Q_INVOKABLE Element* elementAt(int index) const;
    Q_INVOKABLE QList<Element*> getAllElements() const { return m_elements; }
    Q_INVOKABLE QList<Element*> getChildrenRecursive(const QString &parentId) const;
    Q_INVOKABLE void clear();
    Q_INVOKABLE void reorderElement(Element *element, int newIndex);
    Q_INVOKABLE void refresh();  // Force a refresh of the model
    
    // ID generation
    Q_INVOKABLE QString generateId();
    
    // Parent relationship resolution
    Q_INVOKABLE void resolveParentRelationships();
    
    // Instance synchronization
    Q_INVOKABLE void syncInstancesFromSource(Element* sourceElement);
    
signals:
    void elementAdded(Element *element);
    void elementRemoved(const QString &elementId);
    void elementUpdated(Element *element);
    void elementChanged();
    
private slots:
    void onElementChanged();
    void onSourceElementPropertyChanged();
    
private:
    QList<Element*> m_elements;
    QSet<QString> m_connectedSourceElements; // Track which source elements we've connected to
    
    void connectElement(Element *element);
    void disconnectElement(Element *element);
    void connectSourceElement(Element* sourceElement);
    void disconnectSourceElement(Element* sourceElement);
    int findElementIndex(const QString &elementId) const;
    void createChildInstancesForSourceChild(Element* childElement, const QString& sourceParentId);
};