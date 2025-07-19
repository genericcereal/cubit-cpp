#pragma once
#include "DesignElement.h"
#include <QColor>
#include <memory>

class FlexLayoutEngine;

class Frame : public DesignElement {
    Q_OBJECT
    Q_PROPERTY(QColor fill READ fill WRITE setFill NOTIFY fillChanged)
    Q_PROPERTY(ColorFormat colorFormat READ colorFormat WRITE setColorFormat NOTIFY colorFormatChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(OverflowMode overflow READ overflow WRITE setOverflow NOTIFY overflowChanged)
    Q_PROPERTY(bool acceptsChildren READ acceptsChildren WRITE setAcceptsChildren NOTIFY acceptsChildrenChanged)
    Q_PROPERTY(bool flex READ flex WRITE setFlex NOTIFY flexChanged)
    Q_PROPERTY(LayoutOrientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(qreal gap READ gap WRITE setGap NOTIFY gapChanged)
    Q_PROPERTY(PositionType position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(JustifyContent justify READ justify WRITE setJustify NOTIFY justifyChanged)
    Q_PROPERTY(AlignItems align READ align WRITE setAlign NOTIFY alignChanged)
    Q_PROPERTY(SizeType widthType READ widthType WRITE setWidthType NOTIFY widthTypeChanged)
    Q_PROPERTY(SizeType heightType READ heightType WRITE setHeightType NOTIFY heightTypeChanged)
    Q_PROPERTY(bool canResizeWidth READ canResizeWidth NOTIFY canResizeWidthChanged)
    Q_PROPERTY(bool canResizeHeight READ canResizeHeight NOTIFY canResizeHeightChanged)
    Q_PROPERTY(bool controlled READ controlled WRITE setControlled NOTIFY controlledChanged)
    Q_PROPERTY(Role role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(QString platform READ platform WRITE setPlatform NOTIFY platformChanged)
    
public:
    enum Role {
        undefined,
        container
    };
    Q_ENUM(Role)
    enum ColorFormat {
        RGB,
        HEX,
        HSL
    };
    Q_ENUM(ColorFormat)
    
    enum OverflowMode {
        Hidden,
        Scroll,
        Visible
    };
    Q_ENUM(OverflowMode)
    
    enum LayoutOrientation {
        Row,
        Column
    };
    Q_ENUM(LayoutOrientation)
    
    enum PositionType {
        Relative,
        Absolute,
        Fixed
    };
    Q_ENUM(PositionType)
    
    
    enum JustifyContent {
        JustifyStart,
        JustifyEnd,
        JustifyCenter,
        JustifySpaceBetween,
        JustifySpaceAround,
        JustifySpaceEvenly
    };
    Q_ENUM(JustifyContent)
    
    enum AlignItems {
        AlignStart,
        AlignEnd,
        AlignCenter,
        AlignBaseline,
        AlignStretch
    };
    Q_ENUM(AlignItems)
    
    enum SizeType {
        SizeFixed,      // Default - uses fixed pixel values
        SizeRelative,   // Percentage of parent (only when flex is enabled)
        SizeFill,       // Fill available space (only when frame has a parent)
        SizeFitContent, // Size to fit children (only when frame has children)
        SizeViewport    // Percentage of viewport
    };
    Q_ENUM(SizeType)
    
    explicit Frame(const QString &id, QObject *parent = nullptr);
    ~Frame();
    
    // Property getters
    QColor fill() const { return m_fill; }
    ColorFormat colorFormat() const { return m_colorFormat; }
    QColor borderColor() const { return m_borderColor; }
    int borderWidth() const { return m_borderWidth; }
    int borderRadius() const { return m_borderRadius; }
    OverflowMode overflow() const { return m_overflow; }
    bool acceptsChildren() const { return m_acceptsChildren; }
    bool flex() const { return m_flex; }
    LayoutOrientation orientation() const { return m_orientation; }
    qreal gap() const { return m_gap; }
    PositionType position() const { return m_position; }
    JustifyContent justify() const { return m_justify; }
    AlignItems align() const { return m_align; }
    SizeType widthType() const { return m_widthType; }
    SizeType heightType() const { return m_heightType; }
    bool canResizeWidth() const;
    bool canResizeHeight() const;
    bool controlled() const { return m_controlled; }
    Role role() const { return m_role; }
    QString platform() const { return m_platform; }
    
    // Property setters
    void setFill(const QColor &color);
    void setColorFormat(ColorFormat format);
    void setBorderColor(const QColor &color);
    void setBorderWidth(int width);
    void setBorderRadius(int radius);
    void setOverflow(OverflowMode mode);
    void setAcceptsChildren(bool accepts);
    void setFlex(bool flex);
    void setOrientation(LayoutOrientation orientation);
    void setGap(qreal gap);
    void setPosition(PositionType position);
    void setJustify(JustifyContent justify);
    void setAlign(AlignItems align);
    void setWidthType(SizeType type);
    void setHeightType(SizeType type);
    void setControlled(bool controlled);
    void setRole(Role role);
    void setPlatform(const QString& platform);
    
    // Override geometry setters to trigger layout
    void setWidth(qreal w) override;
    void setHeight(qreal h) override;
    void setRect(const QRectF &rect) override;
    
    // Override to provide Frame-specific property definitions
    QList<PropertyDefinition> propertyDefinitions() const override;
    
signals:
    void fillChanged();
    void colorFormatChanged();
    void borderColorChanged();
    void borderWidthChanged();
    void borderRadiusChanged();
    void overflowChanged();
    void acceptsChildrenChanged();
    void flexChanged();
    void orientationChanged();
    void gapChanged();
    void positionChanged();
    void justifyChanged();
    void alignChanged();
    void widthTypeChanged();
    void heightTypeChanged();
    void canResizeWidthChanged();
    void canResizeHeightChanged();
    void controlledChanged();
    void roleChanged();
    void platformChanged();
    
public slots:
    void triggerLayout();
    
private:
    QColor m_fill;
    ColorFormat m_colorFormat = HEX; // Default to HEX
    QColor m_borderColor;
    int m_borderWidth;
    int m_borderRadius;
    OverflowMode m_overflow;
    bool m_acceptsChildren;
    bool m_flex;
    LayoutOrientation m_orientation;
    qreal m_gap;
    PositionType m_position;
    JustifyContent m_justify;
    AlignItems m_align;
    SizeType m_widthType = SizeFixed;
    SizeType m_heightType = SizeFixed;
    bool m_controlled = true; // Default to true
    Role m_role = undefined;
    QString m_platform; // Default to empty string (undefined)
    
    // Layout engine
    std::unique_ptr<class FlexLayoutEngine> m_layoutEngine;
    
    // Setup connections to element model
    void setupElementModelConnections();
};