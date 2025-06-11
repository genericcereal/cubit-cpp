#include <QtTest>
#include <QSignalSpy>
#include "QmlTestHelper.h"
#include "../src/ElementModel.h"
#include "../src/Frame.h"
#include "../src/Text.h"
#include "../src/Html.h"

class tst_ElementModel : public QmlTestHelper
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        // Any global initialization
    }
    
    void init() {
        // Run before each test
        m_model = new ElementModel(this);
    }
    
    void cleanup() {
        // Run after each test
        delete m_model;
        m_model = nullptr;
    }
    
    void test_createElements() {
        // Create different types of elements
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        controller.createElement("text", 10, 10, 80, 20);
        controller.createElement("html", 20, 20, 60, 60);
        
        // Verify model count (text and html each create Frame + child element)
        QCOMPARE(m_model->rowCount(), 5);  // frame (1) + text creates frame + text (2) + html creates frame + html (2) = 5 total
        
        // Get elements and verify they exist
        Element* frame1 = m_model->elementAt(0);  // standalone frame
        Element* frame2 = m_model->elementAt(1);  // frame for text
        Element* text = m_model->elementAt(2);    // text element
        Element* frame3 = m_model->elementAt(3);  // frame for html
        Element* html = m_model->elementAt(4);    // html element
        
        QVERIFY(frame1 != nullptr);
        QVERIFY(frame2 != nullptr);
        QVERIFY(text != nullptr);
        QVERIFY(frame3 != nullptr);
        QVERIFY(html != nullptr);
        
        // Verify types based on actual createElement behavior
        QCOMPARE(frame1->getTypeName(), QString("Frame"));
        QCOMPARE(frame2->getTypeName(), QString("Frame"));
        QCOMPARE(text->getTypeName(), QString("Text"));
        QCOMPARE(frame3->getTypeName(), QString("Frame"));
        QCOMPARE(html->getTypeName(), QString("Html"));
    }
    
    void test_elementOrder() {
        // Create elements in order
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        controller.createElement("frame", 100, 0, 100, 100);
        controller.createElement("frame", 200, 0, 100, 100);
        
        QCOMPARE(m_model->rowCount(), 3);
        
        // Get elements and verify order
        Element* first = m_model->elementAt(0);
        Element* second = m_model->elementAt(1);
        Element* third = m_model->elementAt(2);
        
        QVERIFY(first != nullptr);
        QVERIFY(second != nullptr);
        QVERIFY(third != nullptr);
        
        // Verify positions match creation order
        QCOMPARE(first->x(), 0.0);
        QCOMPARE(second->x(), 100.0);
        QCOMPARE(third->x(), 200.0);
    }
    
    void test_removeElements() {
        // Create elements
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        controller.createElement("frame", 100, 0, 100, 100);
        controller.createElement("frame", 200, 0, 100, 100);
        
        QCOMPARE(m_model->rowCount(), 3);
        
        Element* elem1 = m_model->elementAt(0);
        Element* elem2 = m_model->elementAt(1);
        Element* elem3 = m_model->elementAt(2);
        
        // Remove middle element by ID
        m_model->removeElement(elem2->getId());
        
        QCOMPARE(m_model->rowCount(), 2);
        QCOMPARE(m_model->elementAt(0), elem1);
        QCOMPARE(m_model->elementAt(1), elem3);
    }
    
    void test_clearModel() {
        // Create elements
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        controller.createElement("frame", 100, 0, 100, 100);
        controller.createElement("frame", 200, 0, 100, 100);
        
        QCOMPARE(m_model->rowCount(), 3);
        
        // Clear
        m_model->clear();
        
        QCOMPARE(m_model->rowCount(), 0);
    }
    
    void test_modelSignals() {
        // Set up signal spies
        QSignalSpy rowsInsertedSpy(m_model, &QAbstractItemModel::rowsInserted);
        QSignalSpy rowsRemovedSpy(m_model, &QAbstractItemModel::rowsRemoved);
        QSignalSpy modelResetSpy(m_model, &QAbstractItemModel::modelReset);
        
        // Create element
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        QCOMPARE(rowsInsertedSpy.count(), 1);
        
        // Remove element
        Element* element = m_model->elementAt(0);
        m_model->removeElement(element->getId());
        QCOMPARE(rowsRemovedSpy.count(), 1);
        
        // Clear model
        controller.createElement("frame", 0, 0, 100, 100);
        m_model->clear();
        QCOMPARE(modelResetSpy.count(), 1);
    }
    
    void test_elementProperties() {
        // Create a frame with specific properties
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 50, 75, 200, 150);
        
        Frame* frame = qobject_cast<Frame*>(m_model->elementAt(0));
        QVERIFY(frame != nullptr);
        
        // Test properties
        QCOMPARE(frame->x(), 50);
        QCOMPARE(frame->y(), 75);
        QCOMPARE(frame->width(), 200);
        QCOMPARE(frame->height(), 150);
        
        // Modify properties
        frame->setRect(QRectF(100, 125, 300, 250));
        QCOMPARE(frame->x(), 100);
        QCOMPARE(frame->y(), 125);
        QCOMPARE(frame->width(), 300);
        QCOMPARE(frame->height(), 250);
    }
    
    void test_parentChildRelationships() {
        // Create frame with text (this automatically creates parent-child relationship)
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("text", 10, 10, 100, 20);
        
        QCOMPARE(m_model->rowCount(), 2);  // Frame + Text
        
        Element* frame = m_model->elementAt(0);
        Element* text = m_model->elementAt(1);
        
        // Verify the text has the frame as parent
        QVERIFY(text->hasParent());
        QCOMPARE(text->getParentElementId(), frame->getId());
        QVERIFY(!frame->hasParent());
    }
    
    void test_modelWithQml() {
        // Skip QML test if QtQuick is not available
        QSKIP("QtQuick module not available in test environment");
    }
    
private:
    ElementModel* m_model = nullptr;
};

#include "tst_ElementModel.moc"