#ifndef ELEMENTFILTERPROXY_H
#define ELEMENTFILTERPROXY_H

#include <QSortFilterProxyModel>
#include <QObject>

class Element;

class ElementFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(QObject* editingElement READ editingElement WRITE setEditingElement NOTIFY editingElementChanged)
    Q_PROPERTY(bool filterComponentsOut READ filterComponentsOut WRITE setFilterComponentsOut NOTIFY filterComponentsOutChanged)
    Q_PROPERTY(bool filterComponentsOnly READ filterComponentsOnly WRITE setFilterComponentsOnly NOTIFY filterComponentsOnlyChanged)

public:
    explicit ElementFilterProxy(QObject *parent = nullptr);
    
    // Override roleNames to expose the source model's roles
    QHash<int, QByteArray> roleNames() const override;
    
    // Convenience method to get element at index (for QML)
    Q_INVOKABLE Element* elementAt(int row) const;

    QString viewMode() const;
    void setViewMode(const QString& viewMode);

    QObject* editingElement() const;
    void setEditingElement(QObject* element);

    bool filterComponentsOut() const;
    void setFilterComponentsOut(bool filter);

    bool filterComponentsOnly() const;
    void setFilterComponentsOnly(bool filter);

signals:
    void viewModeChanged();
    void editingElementChanged();
    void filterComponentsOutChanged();
    void filterComponentsOnlyChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool shouldShowElementInMode(Element* element) const;

    QString m_viewMode;
    QObject* m_editingElement = nullptr;
    bool m_filterComponentsOut = false;
    bool m_filterComponentsOnly = false;
};

#endif // ELEMENTFILTERPROXY_H