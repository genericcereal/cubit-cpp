#ifndef CONFIGOBJECT_H
#define CONFIGOBJECT_H

#include <QObject>
#include <QQmlEngine>
#include "Config.h"

// Minimal singleton to expose Config values to QML
class ConfigObject : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    // Just expose the essential values that are actually used in QML
    Q_PROPERTY(int targetFps READ targetFps CONSTANT)
    Q_PROPERTY(int throttleInterval READ throttleInterval CONSTANT)
    Q_PROPERTY(bool adaptiveThrottlingDefault READ adaptiveThrottlingDefault CONSTANT)
    
public:
    ConfigObject(QObject *parent = nullptr) : QObject(parent) {}
    
    int targetFps() const { return Config::TARGET_FPS; }
    int throttleInterval() const { return Config::THROTTLE_INTERVAL; }
    bool adaptiveThrottlingDefault() const { return Config::ADAPTIVE_THROTTLING_DEFAULT; }
};

#endif // CONFIGOBJECT_H