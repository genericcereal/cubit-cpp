#pragma once
#include <QObject>
#include <QMetaProperty>
#include <QStringList>
#include "ConnectionManager.h"

class PropertySyncer {
public:
    static void sync(QObject* source, QObject* target,
                     const QStringList& properties,
                     const char* slotSignature,
                     ConnectionManager& cm) 
    {
        const QMetaObject* sourceMetaObj = source->metaObject();
        const QMetaObject* targetMetaObj = target->metaObject();
        
        int slotIndex = targetMetaObj->indexOfSlot(slotSignature);
        if (slotIndex < 0) {
            qWarning() << "PropertySyncer: Slot" << slotSignature << "not found in target object";
            return;
        }
        
        QMetaMethod slot = targetMetaObj->method(slotIndex);

        for (const QString& propertyName : properties) {
            int propertyIndex = sourceMetaObj->indexOfProperty(propertyName.toUtf8().constData());
            if (propertyIndex < 0) {
                continue;
            }
            
            QMetaProperty property = sourceMetaObj->property(propertyIndex);
            if (!property.hasNotifySignal()) {
                continue;
            }
            
            QMetaMethod notifySignal = property.notifySignal();
            cm.add(QObject::connect(source, notifySignal, target, slot, Qt::UniqueConnection));
        }
    }
};