#ifndef QMLTESTHELPER_H
#define QMLTESTHELPER_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QVariant>
#include <QTest>
#include <memory>

class QmlTestHelper : public QObject
{
    Q_OBJECT
public:
    explicit QmlTestHelper(QObject *parent = nullptr) : QObject(parent) {
        m_engine = std::make_unique<QQmlEngine>(this);
        
        // Set import paths
        m_engine->addImportPath(QStringLiteral("../qml"));
        m_engine->addImportPath(QStringLiteral(".."));
    }
    
    QObject* createQmlObject(const QString& qml, const QString& errorPrefix = "Failed to create QML object") {
        QQmlComponent component(m_engine.get());
        component.setData(qml.toUtf8(), QUrl());
        
        if (component.status() != QQmlComponent::Ready) {
            qWarning() << errorPrefix << ":" << component.errorString();
            return nullptr;
        }
        
        QObject* obj = component.create();
        if (!obj) {
            qWarning() << errorPrefix << ": Component created but object is null";
            return nullptr;
        }
        
        // Parent to this helper to ensure cleanup
        obj->setParent(this);
        return obj;
    }
    
    QVariant getProperty(QObject* obj, const QString& propertyName) {
        if (!obj) return QVariant();
        return obj->property(propertyName.toUtf8().constData());
    }
    
    bool setProperty(QObject* obj, const QString& propertyName, const QVariant& value) {
        if (!obj) return false;
        return obj->setProperty(propertyName.toUtf8().constData(), value);
    }
    
    bool invokeMethod(QObject* obj, const QString& methodName, 
                      QGenericReturnArgument returnValue = QGenericReturnArgument(),
                      QGenericArgument arg1 = QGenericArgument(),
                      QGenericArgument arg2 = QGenericArgument(),
                      QGenericArgument arg3 = QGenericArgument(),
                      QGenericArgument arg4 = QGenericArgument()) {
        if (!obj) return false;
        return QMetaObject::invokeMethod(obj, methodName.toUtf8().constData(), 
                                         Qt::DirectConnection, returnValue, arg1, arg2, arg3, arg4);
    }
    
    QQmlEngine* engine() const { return m_engine.get(); }
    
private:
    std::unique_ptr<QQmlEngine> m_engine;
};

// Macro helpers for common test patterns
#define QVERIFY_PROPERTY(obj, prop, expected) \
    QCOMPARE(getProperty(obj, prop), QVariant(expected))

#define QVERIFY_PROPERTY_TYPE(obj, prop, type) \
    QVERIFY(getProperty(obj, prop).canConvert<type>())

#endif // QMLTESTHELPER_H