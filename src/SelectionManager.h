#pragma once
#include <QObject>
#include <QList>
#include <QSet>
#include <vector>
#include "Element.h"

class SelectionManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(int selectionCount READ selectionCount NOTIFY selectionChanged)
    Q_PROPERTY(QList<Element*> selectedElements READ selectedElements NOTIFY selectionChanged)
    Q_PROPERTY(bool hasVisualSelection READ hasVisualSelection NOTIFY selectionChanged)
    Q_PROPERTY(qreal boundingX READ boundingX NOTIFY selectionChanged)
    Q_PROPERTY(qreal boundingY READ boundingY NOTIFY selectionChanged)
    Q_PROPERTY(qreal boundingWidth READ boundingWidth NOTIFY selectionChanged)
    Q_PROPERTY(qreal boundingHeight READ boundingHeight NOTIFY selectionChanged)
    
public:
    explicit SelectionManager(QObject *parent = nullptr);
    
    // Selection queries
    bool hasSelection() const { return !m_selectedElements.isEmpty(); }
    int selectionCount() const { return m_selectedElements.size(); }
    QList<Element*> selectedElements() const { return m_selectedElements.values(); }
    Q_INVOKABLE bool isSelected(Element *element) const;
    Q_INVOKABLE bool hasVisualSelection() const;
    
    // Bounding box properties
    qreal boundingX() const { return m_boundingX; }
    qreal boundingY() const { return m_boundingY; }
    qreal boundingWidth() const { return m_boundingWidth; }
    qreal boundingHeight() const { return m_boundingHeight; }
    
public slots:
    // Selection manipulation
    void selectElement(Element *element);
    void deselectElement(Element *element);
    void toggleSelection(Element *element);
    void selectOnly(Element *element);
    void selectAll(const std::vector<Element*> &elements);
    void clearSelection();
    
private slots:
    void onElementGeometryChanged();
    
signals:
    void selectionChanged();
    void elementSelected(Element *element);
    void elementDeselected(Element *element);
    
private:
    QSet<Element*> m_selectedElements;
    qreal m_boundingX = 0;
    qreal m_boundingY = 0;
    qreal m_boundingWidth = 0;
    qreal m_boundingHeight = 0;
    
    void updateElementSelection(Element *element, bool selected);
    void recalculateBoundingBox();
    void expandBoundingBox(Element *element);
    void shrinkBoundingBox();
};