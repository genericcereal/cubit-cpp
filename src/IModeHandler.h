#pragma once
#include <QtGlobal>

// Interface for handling mouse events in different canvas modes
struct IModeHandler {
    virtual ~IModeHandler() = default;
    
    virtual void onPress(qreal x, qreal y) = 0;
    virtual void onMove(qreal x, qreal y) = 0;
    virtual void onRelease(qreal x, qreal y) = 0;
};