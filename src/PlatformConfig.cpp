#include "PlatformConfig.h"
#include "Scripts.h"
#include "Node.h"
#include "UniqueIdGenerator.h"
#include "ElementModel.h"
#include "Frame.h"
#include <QDebug>

PlatformConfig::PlatformConfig(Type type, QObject* parent)
    : QObject(parent)
    , m_type(type)
{
    initializeFromType(type);
    
    // Create platform-specific scripts
    m_scripts = std::make_unique<Scripts>(this);
    
    // Create global elements model
    m_globalElements = std::make_unique<ElementModel>(this);
    
    // Create platform-specific onLoad node
    createPlatformOnLoadNode();
    
    // Create initial global elements
    createInitialGlobalElements();
}

PlatformConfig::~PlatformConfig() = default;

QString PlatformConfig::name() const {
    return m_name;
}

Scripts* PlatformConfig::scripts() const {
    return m_scripts.get();
}

QString PlatformConfig::displayName() const {
    return m_displayName;
}

PlatformConfig::Type PlatformConfig::type() const {
    return m_type;
}

ElementModel* PlatformConfig::globalElements() const {
    return m_globalElements.get();
}

PlatformConfig* PlatformConfig::create(const QString& platformName, QObject* parent) {
    Type type;
    QString lowerName = platformName.toLower();
    
    if (lowerName == "web") {
        type = Web;
    } else if (lowerName == "ios") {
        type = iOS;
    } else if (lowerName == "android") {
        type = Android;
    } else {
        qWarning() << "Unknown platform type:" << platformName << "- defaulting to Web";
        type = Web;
    }
    
    return new PlatformConfig(type, parent);
}

void PlatformConfig::initializeFromType(Type type) {
    switch (type) {
        case Web:
            m_name = "web";
            m_displayName = "Web";
            break;
        case iOS:
            m_name = "iOS";
            m_displayName = "iOS";
            break;
        case Android:
            m_name = "android";
            m_displayName = "Android";
            break;
    }
}

void PlatformConfig::createPlatformOnLoadNode() {
    if (!m_scripts) return;
    
    qDebug() << "Creating platform onLoad node for" << m_name;
    
    // Clear any default nodes first
    m_scripts->clear();
    
    // Create the platform-specific onLoad node
    QString nodeId = UniqueIdGenerator::generate16DigitId();
    Node* onLoadNode = new Node(nodeId, m_scripts.get());
    
    // Configure the onLoad node based on platform type (matching NodeCatalog.qml)
    QString nodeTitle;
    switch (m_type) {
        case Web:
            nodeTitle = "On Web Load";
            break;
        case iOS:
            nodeTitle = "On iOS Load";
            break;
        case Android:
            nodeTitle = "On Android Load";
            break;
    }
    
    onLoadNode->setNodeTitle(nodeTitle);
    onLoadNode->setNodeType("Event");
    
    // Set position (centered in a reasonable default location)
    onLoadNode->setX(-100);
    onLoadNode->setY(0);
    onLoadNode->setWidth(150);
    onLoadNode->setHeight(80);
    
    // Configure the node's ports based on the catalog
    // Platform onLoad nodes have one source port: "done" (Flow type)
    Node::RowConfig rowConfig;
    rowConfig.hasSource = true;
    rowConfig.sourceLabel = "Done";
    rowConfig.sourceType = "Flow";
    rowConfig.sourcePortIndex = 0;
    onLoadNode->addRow(rowConfig);
    
    // Add output port
    onLoadNode->addOutputPort("done");
    onLoadNode->setOutputPortType(0, "Flow");
    
    // Add the node to scripts
    m_scripts->addNode(onLoadNode);
}

void PlatformConfig::createInitialGlobalElements() {
    if (!m_globalElements) return;
    
    qDebug() << "Creating initial global elements for" << m_name;
    
    // Create a default frame
    QString frameId = UniqueIdGenerator::generate16DigitId();
    Frame* frame = new Frame(frameId, m_globalElements.get());
    
    // Set frame properties
    frame->setName("Global Frame");
    frame->setX(100);
    frame->setY(100);
    
    // Set the platform for this frame
    frame->setPlatform(m_name);
    
    // Set role to appContainer AFTER platform - this is exclusive to global elements
    // (setPlatform would set it to container by default, so we override it here)
    frame->setRole(Frame::appContainer);
    
    // Set size based on platform type
    if (m_type == iOS) {
        // iPhone viewport size
        frame->setWidth(375);
        frame->setHeight(812);
    } else {
        // Default size for web and Android (for now)
        frame->setWidth(400);
        frame->setHeight(400);
    }
    
    frame->setFill(QColor("#4287f5")); // Blue background
    frame->setBorderColor(QColor("#2563eb")); // Darker blue border
    frame->setBorderWidth(1);
    frame->setBorderRadius(4);
    
    // Add the frame to the global elements model
    m_globalElements->addElement(frame);
}