#ifndef TEXTVARIANT_H
#define TEXTVARIANT_H

#include "Text.h"
#include <QString>

class TextVariant : public Text
{
    Q_OBJECT
    Q_PROPERTY(bool instancesAcceptChildren READ instancesAcceptChildren WRITE setInstancesAcceptChildren NOTIFY instancesAcceptChildrenChanged)
    Q_PROPERTY(QStringList editableProperties READ editableProperties WRITE setEditableProperties NOTIFY editablePropertiesChanged)

public:
    explicit TextVariant(const QString &id, QObject *parent = nullptr);
    ~TextVariant();
    
    // Property getters
    bool instancesAcceptChildren() const { return m_instancesAcceptChildren; }
    QStringList editableProperties() const { return m_editableProperties; }
    
    // Property setters
    void setInstancesAcceptChildren(bool accept);
    void setEditableProperties(const QStringList& properties);

signals:
    void instancesAcceptChildrenChanged();
    void editablePropertiesChanged();

private:
    bool m_instancesAcceptChildren;
    QStringList m_editableProperties;
};

#endif // TEXTVARIANT_H