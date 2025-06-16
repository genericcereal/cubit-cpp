#include <QtTest>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include "QmlTestHelper.h"
#include "../src/CanvasController.h"
#include "../src/ElementModel.h"
#include "../src/SelectionManager.h"
#include "../src/HitTestService.h"
#include "../src/Frame.h"
#include "../src/Node.h"

class tst_HitTestPerformance : public QmlTestHelper
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        // Any global initialization
    }
    
    void init() {
        // Run before each test
        m_model = new ElementModel(this);
        m_selectionManager = new SelectionManager(this);
        m_controller = new CanvasController(this);
        m_controller->setElementModel(m_model);
        m_controller->setSelectionManager(m_selectionManager);
    }
    
    void cleanup() {
        // Run after each test
        delete m_controller;
        delete m_selectionManager;
        delete m_model;
        m_controller = nullptr;
        m_selectionManager = nullptr;
        m_model = nullptr;
    }
    
    void test_hitTestPerformance_designCanvas() {
        // Set canvas type to design
        m_controller->setCanvasType("design");
        
        // Create a bunch of frames for testing
        const int elementCount = 100;
        for (int i = 0; i < elementCount; ++i) {
            qreal x = (i % 10) * 110;
            qreal y = (i / 10) * 110;
            m_controller->createElement("frame", x, y, 100, 100);
        }
        
        QCOMPARE(m_model->rowCount(), elementCount);
        
        // Run the performance test
        runHitTestPerformance();
    }
    
    void test_hitTestPerformance_scriptCanvas() {
        // Set canvas type to script
        m_controller->setCanvasType("script");
        
        // Create a bunch of nodes for testing
        const int nodeCount = 50;
        for (int i = 0; i < nodeCount; ++i) {
            qreal x = (i % 10) * 150;
            qreal y = (i / 10) * 150;
            m_controller->createNode(x, y, QString("Node %1").arg(i));
        }
        
        QCOMPARE(m_model->rowCount(), nodeCount);
        
        // Run the performance test
        runHitTestPerformance();
    }
    
    void test_hitTestPerformance_mixedElements() {
        // Test with mixed element sizes to stress the quadtree
        m_controller->setCanvasType("design");
        
        // Create small elements
        for (int i = 0; i < 50; ++i) {
            qreal x = QRandomGenerator::global()->bounded(1000);
            qreal y = QRandomGenerator::global()->bounded(1000);
            m_controller->createElement("frame", x, y, 20, 20);
        }
        
        // Create medium elements
        for (int i = 0; i < 30; ++i) {
            qreal x = QRandomGenerator::global()->bounded(900);
            qreal y = QRandomGenerator::global()->bounded(900);
            m_controller->createElement("frame", x, y, 100, 100);
        }
        
        // Create large elements
        for (int i = 0; i < 10; ++i) {
            qreal x = QRandomGenerator::global()->bounded(500);
            qreal y = QRandomGenerator::global()->bounded(500);
            m_controller->createElement("frame", x, y, 500, 500);
        }
        
        QCOMPARE(m_model->rowCount(), 90);
        
        // Run the performance test
        runHitTestPerformance();
    }
    
private:
    ElementModel* m_model = nullptr;
    SelectionManager* m_selectionManager = nullptr;
    CanvasController* m_controller = nullptr;
    
    void runHitTestPerformance() {
        // Get the HitTestService from the controller
        // Note: We need to use the controller's hit test method since HitTestService is private
        
        QElapsedTimer timer;
        int hits = 0;
        const int testCount = 1000;
        
        // Get canvas bounds for random point generation
        QList<Element*> elements = m_model->getAllElements();
        QVERIFY(!elements.isEmpty());
        
        // Calculate bounds
        qreal minX = 0, minY = 0, maxX = 1000, maxY = 1000;
        for (Element* elem : elements) {
            if (elem->isVisual()) {
                CanvasElement* ce = qobject_cast<CanvasElement*>(elem);
                if (ce) {
                    minX = qMin(minX, ce->x());
                    minY = qMin(minY, ce->y());
                    maxX = qMax(maxX, ce->x() + ce->width());
                    maxY = qMax(maxY, ce->y() + ce->height());
                }
            }
        }
        
        // Test with quadtree (default)
        timer.start();
        for (int i = 0; i < testCount; ++i) {
            qreal x = minX + (maxX - minX) * QRandomGenerator::global()->generateDouble();
            qreal y = minY + (maxY - minY) * QRandomGenerator::global()->generateDouble();
            if (m_controller->hitTest(x, y)) {
                hits++;
            }
        }
        qint64 quadTreeTime = timer.elapsed();
        int quadTreeHits = hits;
        
        // We can't directly test without quadtree as HitTestService is private
        // But we can verify the performance is reasonable
        qDebug() << "=== Hit Test Performance ===";
        qDebug() << "Element count:" << elements.size();
        qDebug() << "Test count:" << testCount;
        qDebug() << "QuadTree time:" << quadTreeTime << "ms, hits:" << quadTreeHits;
        qDebug() << "Average time per hit test:" << QString::number(double(quadTreeTime) / double(testCount), 'f', 3) << "ms";
        
        // Performance assertions
        // QuadTree should be fast enough - average less than 0.5ms per hit test
        double avgTimePerTest = double(quadTreeTime) / double(testCount);
        QVERIFY2(avgTimePerTest < 0.5, qPrintable(QString("Hit test too slow: %1ms average").arg(avgTimePerTest)));
        
        // Sanity check - we should have some hits
        QVERIFY(quadTreeHits > 0);
        QVERIFY(quadTreeHits < testCount); // And some misses
    }
};

#include "tst_HitTestPerformance.moc"

QTEST_MAIN(tst_HitTestPerformance)