#include "ScriptCompiler.h"
#include "ScriptGraphValidator.h"
#include "ScriptInvokeBuilder.h"
#include "ScriptFunctionRegistry.h"
#include "ScriptSerializer.h"
#include "Scripts.h"
#include "Node.h"
#include <QDebug>

ScriptCompiler::ScriptCompiler(QObject *parent) 
    : QObject(parent)
    , m_validator(std::make_unique<ScriptGraphValidator>())
    , m_invokeBuilder(std::make_unique<ScriptInvokeBuilder>())
    , m_functionRegistry(std::make_unique<ScriptFunctionRegistry>())
    , m_serializer(std::make_unique<ScriptSerializer>())
{
    // Set up the serializer with the function registry
    m_serializer->setFunctionRegistry(m_functionRegistry.get());
}

ScriptCompiler::~ScriptCompiler() = default;

QString ScriptCompiler::compile(Scripts* scripts)
{
    if (!scripts) {
        m_lastError = "No scripts object provided";
        return QString();
    }
    
    // Step 1: Validate the graph
    if (!m_validator->validate(scripts)) {
        m_lastError = m_validator->getLastError();
        return QString();
    }
    
    // Step 2: Find all event nodes and build invokes for each
    QMap<QString, ScriptInvokeBuilder::BuildContext> eventContexts;
    
    QList<Node*> nodes = scripts->getAllNodes();
    for (Node* node : nodes) {
        if (node && node->nodeType() == "Event") {
            // Normalize event name
            QString eventName = node->nodeTitle().toLower().remove(' ');
            
            // Build invokes for this event
            ScriptInvokeBuilder::BuildContext context = m_invokeBuilder->buildInvokes(node, scripts);
            
            // Store context for this event
            eventContexts[eventName] = context;
        }
    }
    
    // Step 3: Serialize to JSON
    QJsonDocument doc = m_serializer->serialize(eventContexts, scripts);
    
    m_lastError.clear();
    return doc.toJson(QJsonDocument::Compact);
}

QString ScriptCompiler::getLastError() const
{
    return m_lastError;
}