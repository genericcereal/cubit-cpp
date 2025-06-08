#include "CanvasController.h"
#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Html.h"
#include "Variable.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "Config.h"
#include <QDebug>

CanvasController::CanvasController(QObject *parent)
    : QObject(parent)
    , m_mode("select")
    , m_elementModel(nullptr)
    , m_selectionManager(nullptr)
    , m_isDragging(false)
    , m_dragElement(nullptr)
{
}

void CanvasController::setMode(const QString &mode)
{
    qDebug() << "CanvasController::setMode called - current:" << m_mode << "new:" << mode;
    if (m_mode != mode) {
        m_mode = mode;
        qDebug() << "Mode changed, emitting signal";
        emit modeChanged();
    }
}

void CanvasController::handleMousePress(qreal x, qreal y)
{
    if (!m_elementModel || !m_selectionManager) return;
    
    if (m_mode == "select") {
        Element *element = hitTest(x, y);
        if (element) {
            m_selectionManager->selectOnly(element);
            startDrag(x, y);
        } else {
            m_selectionManager->clearSelection();
        }
    } else {
        // Start element creation
        m_dragStartPos = QPointF(x, y);
        m_isDragging = true;
        
        // Create element immediately for frame mode
        if (m_mode == "frame") {
            createElement(m_mode, x, y, 1, 1);
            // The last created element becomes the drag element
            if (m_elementModel && m_elementModel->rowCount() > 0) {
                m_dragElement = m_elementModel->elementAt(m_elementModel->rowCount() - 1);
                qDebug() << "Frame created, dragElement:" << m_dragElement << "isDragging:" << m_isDragging;
            }
        }
    }
}

void CanvasController::handleMouseMove(qreal x, qreal y)
{
    if (!m_isDragging) return;
    
    if (m_dragElement) {
        if (m_mode == "select") {
            updateDrag(x, y);
        } else if (m_mode == "frame") {
            // Update frame size during creation
            qreal width = qAbs(x - m_dragStartPos.x());
            qreal height = qAbs(y - m_dragStartPos.y());
            qreal left = qMin(x, m_dragStartPos.x());
            qreal top = qMin(y, m_dragStartPos.y());
            
            m_dragElement->setX(left);
            m_dragElement->setY(top);
            m_dragElement->setWidth(qMax(width, 1.0));
            m_dragElement->setHeight(qMax(height, 1.0));
            
            qDebug() << "Frame resize:" << left << top << width << height;
        }
    } else if (m_mode != "select") {
        m_dragCurrentPos = QPointF(x, y);
    }
}

void CanvasController::handleMouseRelease(qreal x, qreal y)
{
    if (!m_isDragging) return;
    
    if (m_mode == "select") {
        endDrag();
    } else if (m_mode == "frame") {
        // Finalize frame creation
        if (m_dragElement) {
            // Ensure minimum size
            if (m_dragElement->width() < 10) m_dragElement->setWidth(Config::DEFAULT_ELEMENT_WIDTH);
            if (m_dragElement->height() < 10) m_dragElement->setHeight(Config::DEFAULT_ELEMENT_HEIGHT);
            
            // Select the newly created frame
            if (m_selectionManager) {
                m_selectionManager->selectOnly(m_dragElement);
            }
            
            // Switch back to select mode
            setMode("select");
        }
    } else {
        // Create other element types on release
        qreal width = qAbs(x - m_dragStartPos.x());
        qreal height = qAbs(y - m_dragStartPos.y());
        qreal left = qMin(x, m_dragStartPos.x());
        qreal top = qMin(y, m_dragStartPos.y());
        
        if (width < 10) width = Config::DEFAULT_ELEMENT_WIDTH;
        if (height < 10) height = Config::DEFAULT_ELEMENT_HEIGHT;
        
        createElement(m_mode, left, top, width, height);
    }
    
    m_isDragging = false;
    m_dragElement = nullptr;
}

void CanvasController::createElement(const QString &type, qreal x, qreal y, qreal width, qreal height)
{
    if (!m_elementModel) return;
    
    int id = m_elementModel->generateId();
    Element *element = nullptr;
    
    if (type == "frame") {
        element = new Frame(id);
    } else if (type == "text") {
        // Create frame with text
        Frame *frame = new Frame(id);
        frame->setRect(QRectF(x, y, width, height));
        m_elementModel->addElement(frame);
        
        int textId = m_elementModel->generateId();
        Text *text = new Text(textId);
        text->setParentElementId(id);
        text->setRect(QRectF(0, 0, width, height));
        element = text;
    } else if (type == "html") {
        // Create frame with html
        Frame *frame = new Frame(id);
        frame->setRect(QRectF(x, y, width, height));
        m_elementModel->addElement(frame);
        
        int htmlId = m_elementModel->generateId();
        Html *html = new Html(htmlId);
        html->setParentElementId(id);
        html->setRect(QRectF(0, 0, width, height));
        element = html;
    } else if (type == "variable") {
        element = new Variable(id);
    }
    
    if (element) {
        element->setRect(QRectF(x, y, width, height));
        m_elementModel->addElement(element);
        emit elementCreated(element);
        
        // Select the newly created element
        if (m_selectionManager) {
            m_selectionManager->selectOnly(element);
        }
    }
}

Element* CanvasController::hitTest(qreal x, qreal y)
{
    if (!m_elementModel) return nullptr;
    
    // Test in reverse order (top to bottom)
    QList<Element*> elements = m_elementModel->getAllElements();
    for (int i = elements.size() - 1; i >= 0; --i) {
        Element *element = elements[i];
        if (element->rect().contains(QPointF(x, y))) {
            return element;
        }
    }
    
    return nullptr;
}

void CanvasController::startDrag(qreal x, qreal y)
{
    m_dragElement = hitTest(x, y);
    if (m_dragElement) {
        m_isDragging = true;
        // Store the offset from the click point to the element's top-left
        m_dragStartPos = QPointF(x - m_dragElement->x(), y - m_dragElement->y());
    }
}

void CanvasController::updateDrag(qreal x, qreal y)
{
    if (m_dragElement && m_isDragging) {
        // Calculate new position maintaining the original offset
        m_dragElement->setX(x - m_dragStartPos.x());
        m_dragElement->setY(y - m_dragStartPos.y());
    }
}

void CanvasController::endDrag()
{
    m_isDragging = false;
    m_dragElement = nullptr;
}

void CanvasController::selectElementsInRect(const QRectF &rect)
{
    if (!m_elementModel || !m_selectionManager) return;
    
    QList<Element*> elementsToSelect;
    QList<Element*> allElements = m_elementModel->getAllElements();
    
    for (Element *element : allElements) {
        if (rect.intersects(element->rect())) {
            elementsToSelect.append(element);
        }
    }
    
    if (!elementsToSelect.isEmpty()) {
        m_selectionManager->selectAll(elementsToSelect);
    }
}

void CanvasController::selectAll()
{
    if (!m_elementModel || !m_selectionManager) return;
    
    m_selectionManager->selectAll(m_elementModel->getAllElements());
}

void CanvasController::deleteSelectedElements()
{
    if (!m_elementModel || !m_selectionManager) return;
    
    QList<Element*> selectedElements = m_selectionManager->selectedElements();
    
    // Clear selection first
    m_selectionManager->clearSelection();
    
    // Delete elements
    for (Element *element : selectedElements) {
        m_elementModel->removeElement(element->getId());
    }
}