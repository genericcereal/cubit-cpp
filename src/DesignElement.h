#ifndef DESIGNELEMENT_H
#define DESIGNELEMENT_H

#include "CanvasElement.h"
#include <memory>
#include <QString>
#include <QVariantMap>

class Scripts;
Q_DECLARE_OPAQUE_POINTER(Scripts*)

class DesignElement : public CanvasElement
{
    Q_OBJECT
    Q_PROPERTY(Scripts* scripts READ scripts NOTIFY scriptsChanged)
    
    // Anchor-based positioning properties
    Q_PROPERTY(qreal left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(qreal right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(qreal top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(qreal bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    
    // Anchor states
    Q_PROPERTY(bool leftAnchored READ leftAnchored WRITE setLeftAnchored NOTIFY leftAnchoredChanged)
    Q_PROPERTY(bool rightAnchored READ rightAnchored WRITE setRightAnchored NOTIFY rightAnchoredChanged)
    Q_PROPERTY(bool topAnchored READ topAnchored WRITE setTopAnchored NOTIFY topAnchoredChanged)
    Q_PROPERTY(bool bottomAnchored READ bottomAnchored WRITE setBottomAnchored NOTIFY bottomAnchoredChanged)
    
    // Frozen state
    Q_PROPERTY(bool isFrozen READ isFrozen WRITE setIsFrozen NOTIFY isFrozenChanged)
    
    // Global element tracking
    Q_PROPERTY(QString globalElementSourceId READ globalElementSourceId WRITE setGlobalElementSourceId NOTIFY globalElementSourceIdChanged)
    
public:
    explicit DesignElement(const QString &id, QObject *parent = nullptr);
    virtual ~DesignElement();
    
    // Override to identify as design element
    virtual bool isDesignElement() const override { return true; }
    virtual bool isScriptElement() const override { return false; }
    
    // Check if this is a component variant or instance
    Q_INVOKABLE virtual bool isComponentVariant() const { return false; }
    Q_INVOKABLE virtual bool isComponentInstance() const { return false; }
    
    // Scripts management
    virtual Scripts* scripts() const;
    
    // Execute a script event with optional data
    Q_INVOKABLE virtual void executeScriptEvent(const QString& eventName, const QVariantMap& eventData = QVariantMap());
    
    // Anchor position getters
    qreal left() const { return m_left; }
    qreal right() const { return m_right; }
    qreal top() const { return m_top; }
    qreal bottom() const { return m_bottom; }
    
    // Anchor position setters
    void setLeft(qreal value);
    void setRight(qreal value);
    void setTop(qreal value);
    void setBottom(qreal value);
    
    // Anchor state getters
    bool leftAnchored() const { return m_leftAnchored; }
    bool rightAnchored() const { return m_rightAnchored; }
    bool topAnchored() const { return m_topAnchored; }
    bool bottomAnchored() const { return m_bottomAnchored; }
    
    // Anchor state setters
    void setLeftAnchored(bool anchored);
    void setRightAnchored(bool anchored);
    void setTopAnchored(bool anchored);
    void setBottomAnchored(bool anchored);
    
    // Frozen state
    bool isFrozen() const { return m_isFrozen; }
    void setIsFrozen(bool frozen);
    
    // Global element tracking
    QString globalElementSourceId() const { return m_globalElementSourceId; }
    void setGlobalElementSourceId(const QString& sourceId);
    
    // Update layout based on parent changes
    void updateFromParentGeometry();
    
    // Update anchor values based on current position
    void updateAnchorsFromGeometry();
    
    // Force recalculation of anchor values (public for external calls)
    Q_INVOKABLE void recalculateAnchors();
    
    // Override geometry setters to update anchors
    void setX(qreal x) override;
    void setY(qreal y) override;
    void setWidth(qreal w) override;
    void setHeight(qreal h) override;
    void setRect(const QRectF &rect) override;
    
    // Override parent setter to initialize anchors
    void setParentElement(CanvasElement* parent) override;
    
    // Overloaded setter with position
    Q_INVOKABLE void setParentElement(CanvasElement* parent, qreal left, qreal top);
    
    // Create a component from this design element
    Q_INVOKABLE class Component* createComponent();
    
    // Static utility to copy properties between elements
    static void copyElementProperties(CanvasElement* target, CanvasElement* source, bool copyGeometry = false);
    
    // Get property definitions for this element type
    virtual QList<class PropertyDefinition> propertyDefinitions() const;
    
    // Register DesignElement properties
    void registerProperties() override;
    
signals:
    void scriptsChanged();
    void leftChanged();
    void rightChanged();
    void topChanged();
    void bottomChanged();
    void leftAnchoredChanged();
    void rightAnchoredChanged();
    void topAnchoredChanged();
    void bottomAnchoredChanged();
    void isFrozenChanged();
    void globalElementSourceIdChanged();

private slots:
    void onParentGeometryChanged();
    
protected:
    std::unique_ptr<Scripts> m_scripts;
    void initializeScripts(bool isComponentInstance = false);
    
private:
    // Helper for component creation
    CanvasElement* copyElementRecursively(CanvasElement* sourceElement, CanvasElement* parentInVariant, class ElementModel* elementModel, QHash<QString, QString>& oldToNewIdMap);
    
    // Anchor positions (relative to parent)
    qreal m_left = 0;
    qreal m_right = 0;
    qreal m_top = 0;
    qreal m_bottom = 0;
    
    // Anchor states
    bool m_leftAnchored = false;
    bool m_rightAnchored = false;
    bool m_topAnchored = false;
    bool m_bottomAnchored = false;
    
    // Flag to prevent circular updates
    bool m_updatingFromAnchors = false;
    
    // Frozen state
    bool m_isFrozen = false;
    
    // Global element tracking
    QString m_globalElementSourceId;
};

#endif // DESIGNELEMENT_H