#pragma once
#include <QObject>
#include <QList>
#include "Element.h"

class SelectionManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(int selectionCount READ selectionCount NOTIFY selectionChanged)
    Q_PROPERTY(QList<Element*> selectedElements READ selectedElements NOTIFY selectionChanged)
    
public:
    explicit SelectionManager(QObject *parent = nullptr);
    
    // Selection queries
    bool hasSelection() const { return !m_selectedElements.isEmpty(); }
    int selectionCount() const { return m_selectedElements.size(); }
    QList<Element*> selectedElements() const { return m_selectedElements; }
    bool isSelected(Element *element) const;
    
public slots:
    // Selection manipulation
    void selectElement(Element *element);
    void deselectElement(Element *element);
    void toggleSelection(Element *element);
    void selectOnly(Element *element);
    void selectAll(const QList<Element*> &elements);
    void clearSelection();
    
signals:
    void selectionChanged();
    void elementSelected(Element *element);
    void elementDeselected(Element *element);
    
private:
    QList<Element*> m_selectedElements;
    
    void updateElementSelection(Element *element, bool selected);
};