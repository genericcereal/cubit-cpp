#pragma once
#include <QString>

// Handle types for node connections
namespace HandleType {
    const QString Flow = "Flow";
    const QString Variable = "Variable";
    
    // Validate if two handle types can connect
    inline bool canConnect(const QString &sourceType, const QString &targetType) {
        // Handle types can only connect to the same type
        return sourceType == targetType;
    }
}