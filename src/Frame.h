#pragma once
#include "DesignElement.h"
#include <QColor>
#include <memory>

class FlexLayoutEngine;

class Frame : public DesignElement {
    Q_OBJECT
    Q_PROPERTY(FillColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(QColor fill READ fill NOTIFY fillColorChanged)
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
    
public:
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
    
    enum FillColor {
        LightBlue,
        DarkBlue,
        Green,
        Red
    };
    Q_ENUM(FillColor)
    
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
    explicit Frame(const QString &id, QObject *parent = nullptr);
    ~Frame();
    
    // Property getters
    FillColor fillColor() const { return m_fillColor; }
    QColor fill() const;
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
    
    // Property setters
    void setFillColor(FillColor color);
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
    
    // Override geometry setters to trigger layout
    void setWidth(qreal w) override;
    void setHeight(qreal h) override;
    void setRect(const QRectF &rect) override;
    
signals:
    void fillColorChanged();
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
    
public slots:
    void layoutChildren();
    
private:
    FillColor m_fillColor;
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
    
    // Layout engine
    std::unique_ptr<class FlexLayoutEngine> m_layoutEngine;
    
    // Helper to trigger layout
    void triggerLayout();
    
    // Setup connections to element model
    void setupElementModelConnections();
};