#include <QtTest>
#include <QSignalSpy>
#include "QmlTestHelper.h"
#include "../src/ElementModel.h"
#include "../src/Frame.h"
#include "../src/Element.h"

class tst_FrameOperations : public QmlTestHelper
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
    
    void test_createFrame() {
        // Create a frame using CanvasController
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 100, 100, 200, 150);
        
        // Verify frame was created and added to model
        QCOMPARE(m_model->rowCount(), 1);
        
        Element* element = m_model->elementAt(0);
        QVERIFY(element != nullptr);
        Frame* frame = qobject_cast<Frame*>(element);
        QVERIFY(frame != nullptr);
        
        QCOMPARE(frame->getTypeName(), QString("Frame"));
        QCOMPARE(frame->x(), 100);
        QCOMPARE(frame->y(), 100);
        QCOMPARE(frame->width(), 200);
        QCOMPARE(frame->height(), 150);
    }
    
    void test_modifyFrameProperties() {
        // Create a frame
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 50, 50, 100, 100);
        
        Frame* frame = qobject_cast<Frame*>(m_model->elementAt(0));
        QVERIFY(frame != nullptr);
        
        // Modify properties
        frame->setX(200);
        frame->setY(150);
        frame->setWidth(300);
        frame->setHeight(250);
        
        // Verify changes
        QCOMPARE(frame->x(), 200);
        QCOMPARE(frame->y(), 150);
        QCOMPARE(frame->width(), 300);
        QCOMPARE(frame->height(), 250);
    }
    
    void test_frameSignals() {
        // Create a frame
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        
        Frame* frame = qobject_cast<Frame*>(m_model->elementAt(0));
        QVERIFY(frame != nullptr);
        
        // Set up signal spy for geometryChanged
        QSignalSpy geometrySpy(frame, &Element::geometryChanged);
        QVERIFY(geometrySpy.isValid());
        
        // Change position
        frame->setX(50);
        QCOMPARE(geometrySpy.count(), 1);
        
        // Change size
        frame->setWidth(200);
        QCOMPARE(geometrySpy.count(), 2);
        
        // Change multiple properties using setRect
        frame->setRect(QRectF(100, 100, 150, 150));
        QCOMPARE(geometrySpy.count(), 3);
    }
    
    void test_deleteFrame() {
        // Create multiple frames
        CanvasController controller;
        controller.setElementModel(m_model);
        controller.createElement("frame", 0, 0, 100, 100);
        controller.createElement("frame", 200, 0, 100, 100);
        controller.createElement("frame", 0, 200, 100, 100);
        
        QCOMPARE(m_model->rowCount(), 3);
        
        Frame* frame1 = qobject_cast<Frame*>(m_model->elementAt(0));
        Frame* frame2 = qobject_cast<Frame*>(m_model->elementAt(1));
        Frame* frame3 = qobject_cast<Frame*>(m_model->elementAt(2));
        
        // Delete the middle frame by ID
        m_model->removeElement(frame2->getId());
        
        QCOMPARE(m_model->rowCount(), 2);
        QCOMPARE(m_model->elementAt(0), frame1);
        QCOMPARE(m_model->elementAt(1), frame3);
    }
    
    void test_frameWithQml() {
        // Skip QML test if QtQuick is not available
        QSKIP("QtQuick module not available in test environment");
    }
    
private:
    ElementModel* m_model = nullptr;
};

#include "tst_FrameOperations.moc"