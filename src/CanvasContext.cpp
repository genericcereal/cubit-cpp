#include "CanvasContext.h"
#include "CanvasController.h"

CanvasContext::CanvasContext(QObject *parent)
    : QObject(parent)
{
}

CanvasContext::~CanvasContext() = default;

CanvasController::CanvasType CanvasContext::getCanvasType() const
{
    // Default mapping based on context type
    QString type = contextType();
    if (type == "script") {
        return CanvasController::CanvasType::Script;
    }
    return CanvasController::CanvasType::Design;
}