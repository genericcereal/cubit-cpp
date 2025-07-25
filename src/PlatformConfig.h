#ifndef PLATFORMCONFIG_H
#define PLATFORMCONFIG_H

#include <QObject>
#include <QString>
#include <memory>
#include "Scripts.h"
#include "ElementModel.h"

class PlatformConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(Scripts* scripts READ scripts CONSTANT)
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    Q_PROPERTY(ElementModel* globalElements READ globalElements CONSTANT)

public:
    enum Type {
        Web,
        iOS,
        Android
    };
    Q_ENUM(Type)

    explicit PlatformConfig(Type type, QObject* parent = nullptr);
    ~PlatformConfig();

    // Property getters
    QString name() const;
    Scripts* scripts() const;
    QString displayName() const;
    Type type() const;
    ElementModel* globalElements() const;

    // Static factory method
    static PlatformConfig* create(const QString& platformName, QObject* parent = nullptr);

signals:
    void scriptsChanged();

private:
    Type m_type;
    QString m_name;
    QString m_displayName;
    std::unique_ptr<Scripts> m_scripts;
    std::unique_ptr<ElementModel> m_globalElements;

    void initializeFromType(Type type);
    void createPlatformOnLoadNode();
    void createInitialGlobalElements();
};

#endif // PLATFORMCONFIG_H