#include <QtTest>
#include <QSignalSpy>
#include "QmlTestHelper.h"
#include "../src/SelectionManager.h"
#include "../src/ElementModel.h"
#include "../src/Frame.h"

class tst_SelectionManager : public QmlTestHelper
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        // Any global initialization
    }
    
    void init() {
        // Run before each test
        m_model = new ElementModel(this);
        m_selection = new SelectionManager(this);
    }
    
    void cleanup() {
        // Run after each test
        delete m_selection;
        delete m_model;
        m_selection = nullptr;
        m_model = nullptr;
    }
    
    void test_singleSelection() {
        // Create a frame
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        
        Frame* frame = qobject_cast<Frame*>(m_model->elementAt(0));
        QVERIFY(frame != nullptr);
        
        // Select it
        m_selection->selectElement(frame);
        
        // Verify selection
        QVERIFY(m_selection->hasSelection());
        QCOMPARE(m_selection->selectedElements().size(), 1);
        QCOMPARE(m_selection->selectedElements().first(), frame);
        QVERIFY(frame->isSelected());
    }
    
    void test_multipleSelection() {
        // Create multiple frames
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        controller.createElement("frame", 200, 0, 100, 100);
        controller.createElement("frame", 0, 200, 100, 100);
        
        Frame* frame1 = qobject_cast<Frame*>(m_model->elementAt(0));
        Frame* frame2 = qobject_cast<Frame*>(m_model->elementAt(1));
        Frame* frame3 = qobject_cast<Frame*>(m_model->elementAt(2));
        
        // Select all
        m_selection->selectElement(frame1);
        m_selection->selectElement(frame2);
        m_selection->selectElement(frame3);
        
        // Verify selection
        QVERIFY(m_selection->hasSelection());
        QCOMPARE(m_selection->selectedElements().size(), 3);
        QVERIFY(frame1->isSelected());
        QVERIFY(frame2->isSelected());
        QVERIFY(frame3->isSelected());
    }
    
    void test_clearSelection() {
        // Create and select frames
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        controller.createElement("frame", 200, 0, 100, 100);
        
        Frame* frame1 = qobject_cast<Frame*>(m_model->elementAt(0));
        Frame* frame2 = qobject_cast<Frame*>(m_model->elementAt(1));
        
        m_selection->selectElement(frame1);
        m_selection->selectElement(frame2);
        
        // Clear selection
        m_selection->clearSelection();
        
        // Verify
        QVERIFY(!m_selection->hasSelection());
        QCOMPARE(m_selection->selectedElements().size(), 0);
        QVERIFY(!frame1->isSelected());
        QVERIFY(!frame2->isSelected());
    }
    
    void test_selectionSignals() {
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        
        Frame* frame = qobject_cast<Frame*>(m_model->elementAt(0));
        
        // Set up signal spy
        QSignalSpy selectionChangedSpy(m_selection, &SelectionManager::selectionChanged);
        QVERIFY(selectionChangedSpy.isValid());
        
        // Select
        m_selection->selectElement(frame);
        QCOMPARE(selectionChangedSpy.count(), 1);
        
        // Clear
        m_selection->clearSelection();
        QCOMPARE(selectionChangedSpy.count(), 2);
    }
    
    void test_selectionBounds() {
        // Create frames at known positions
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 10, 20, 100, 50);
        controller.createElement("frame", 50, 40, 80, 60);
        
        Frame* frame1 = qobject_cast<Frame*>(m_model->elementAt(0));
        Frame* frame2 = qobject_cast<Frame*>(m_model->elementAt(1));
        
        // Select both
        m_selection->selectElement(frame1);
        m_selection->selectElement(frame2);
        
        // Get bounds using individual properties
        qreal left = m_selection->boundingX();
        qreal top = m_selection->boundingY();
        qreal width = m_selection->boundingWidth();
        qreal height = m_selection->boundingHeight();
        
        // Verify bounds encompass both frames
        QCOMPARE(left, 10.0);   // leftmost edge of frame1
        QCOMPARE(top, 20.0);     // topmost edge of frame1
        QCOMPARE(width, 120.0);  // width from x=10 to x=130 (50 + 80)
        QCOMPARE(height, 80.0);  // height from y=20 to y=100 (40 + 60)
    }
    
    void test_selectionWithQml() {
        // Skip QML test if QtQuick is not available
        QSKIP("QtQuick module not available in test environment");
    }
    
private:
    ElementModel* m_model = nullptr;
    SelectionManager* m_selection = nullptr;
};

#include "tst_SelectionManager.moc"