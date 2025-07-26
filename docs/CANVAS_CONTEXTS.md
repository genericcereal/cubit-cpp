# Canvas Context System

## Overview

The Canvas Context system provides a modular way to display different sets of elements on the canvas without hardcoding specific view modes. This allows for easy extension when new contexts are needed (e.g., linked objects, reference elements, library items, etc.).

## Architecture

### Base Class: CanvasContext

All canvas contexts inherit from `CanvasContext` which provides:
- Context identification (`contextType`, `displayName`)
- Element activation/deactivation
- Optional scripts provisioning
- Viewport preferences
- Hit testing configuration

### Built-in Contexts

1. **MainCanvasContext** - The default design canvas showing project elements
2. **VariantCanvasContext** - Shows component variants when editing a component
3. **GlobalElementsContext** - Shows platform-specific global elements
4. **ScriptCanvasContext** - Shows nodes and edges for visual programming

## Usage in Project Class

Instead of the current approach with hardcoded view modes:

```cpp
// OLD APPROACH
void Project::setViewMode(const QString& viewMode) {
    if (viewMode == "script") {
        loadScriptsIntoElementModel();
    } else if (viewMode == "globalElements") {
        loadGlobalElementsIntoModel();
    }
    // ... many if/else conditions
}
```

The new approach uses contexts:

```cpp
// NEW APPROACH
void Project::setCanvasContext(std::unique_ptr<CanvasContext> context) {
    // Deactivate current context
    if (m_currentContext) {
        m_currentContext->deactivateContext(m_elementModel.get());
    }
    
    // Activate new context
    m_currentContext = std::move(context);
    if (m_currentContext) {
        m_currentContext->activateContext(m_elementModel.get());
        
        // Update canvas controller type
        if (m_controller) {
            m_controller->setCanvasType(m_currentContext->getCanvasType());
        }
        
        // Configure hit testing
        if (m_controller && m_controller->hitTestService()) {
            m_currentContext->configureHitTestService(m_controller->hitTestService());
        }
    }
    
    emit canvasContextChanged();
}
```

## Creating New Contexts

To add a new type of canvas context:

1. Create a new class inheriting from `CanvasContext`:

```cpp
class LinkedObjectsContext : public CanvasContext {
    Q_OBJECT
public:
    explicit LinkedObjectsContext(Element* sourceElement, QObject* parent = nullptr);
    
    QString contextType() const override { return "linkedObjects"; }
    QString displayName() const override;
    
    void activateContext(ElementModel* targetModel) override;
    void deactivateContext(ElementModel* targetModel) override;
    
private:
    QPointer<Element> m_sourceElement;
    QStringList m_linkedElementIds;
};
```

2. Implement the activation/deactivation logic:

```cpp
void LinkedObjectsContext::activateContext(ElementModel* targetModel) {
    // Find and add linked elements to the model
    auto linkedElements = findLinkedElements(m_sourceElement);
    for (Element* element : linkedElements) {
        m_linkedElementIds.append(element->getId());
        targetModel->addElement(element);
    }
}

void LinkedObjectsContext::deactivateContext(ElementModel* targetModel) {
    // Remove the linked elements
    for (const QString& id : m_linkedElementIds) {
        if (Element* element = targetModel->getElementById(id)) {
            targetModel->removeElementWithoutDelete(element);
        }
    }
    m_linkedElementIds.clear();
}
```

3. Use the context:

```cpp
// Switch to linked objects view
auto context = std::make_unique<LinkedObjectsContext>(selectedElement);
project->setCanvasContext(std::move(context));
```

## Benefits

1. **Modularity**: Each context is self-contained with its own activation/deactivation logic
2. **Extensibility**: New contexts can be added without modifying existing code
3. **Testability**: Each context can be tested independently
4. **Clarity**: Clear separation of concerns for different view modes
5. **Reusability**: Contexts can be reused across different projects or canvases

## QML Integration

The canvas context is exposed to QML:

```qml
// Access current context info
Text {
    text: canvas.currentContext ? canvas.currentContext.displayName : "No Context"
}

// Check context type
visible: canvas.currentContext && canvas.currentContext.contextType === "variant"
```

## Migration Path

1. Keep existing `viewMode` property for backward compatibility
2. Create contexts internally when `setViewMode` is called
3. Gradually migrate UI code to use contexts directly
4. Eventually deprecate `viewMode` in favor of contexts