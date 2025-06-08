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
    Q_INVOKABLE void removeElement(int elementId);
    Q_INVOKABLE Element* getElementById(int elementId) const;
    Q_INVOKABLE Element* elementAt(int index) const;
    Q_INVOKABLE QList<Element*> getAllElements() const { return m_elements; }
    Q_INVOKABLE void clear();
    
    // ID generation
    Q_INVOKABLE int generateId();
    
signals:
    void elementAdded(Element *element);
    void elementRemoved(int elementId);
    void elementUpdated(Element *element);
    
private slots:
    void onElementChanged();
    
private:
    QList<Element*> m_elements;
    int m_nextId;
    
    void connectElement(Element *element);
    void disconnectElement(Element *element);
    int findElementIndex(int elementId) const;
};