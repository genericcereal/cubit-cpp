#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QThread>
#include <QMouseEvent>
#include <QPainter>
#include <memory>
#include <vector>
#include <numeric>
#include <iomanip>
#include <iostream>

#include "Canvas.h"
#include "GLCanvas.h"

class BenchmarkCanvas : public Canvas {
public:
    BenchmarkCanvas() : Canvas() {}
    
    // Expose paintEvent for benchmarking
    void benchmarkPaint() {
        QPaintEvent event(rect());
        paintEvent(&event);
    }
    
    // Expose mouse events for benchmarking
    void simulateMousePress(const QPoint& pos) {
        QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mousePressEvent(&event);
    }
    
    void simulateMouseMove(const QPoint& pos) {
        QMouseEvent event(QEvent::MouseMove, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseMoveEvent(&event);
    }
};

class BenchmarkGLCanvas : public GLCanvas {
public:
    BenchmarkGLCanvas() : GLCanvas() {}
    
    // Expose paintGL for benchmarking
    void benchmarkPaint() {
        makeCurrent();
        paintGL();
        doneCurrent();
    }
    
    // Expose mouse events for benchmarking
    void simulateMousePress(const QPoint& pos) {
        QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mousePressEvent(&event);
    }
    
    void simulateMouseMove(const QPoint& pos) {
        QMouseEvent event(QEvent::MouseMove, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseMoveEvent(&event);
    }
};

class BenchmarkRunner {
public:
    struct BenchmarkResult {
        QString name;
        double minTime;
        double maxTime;
        double avgTime;
        double medianTime;
        int iterations;
    };

    static void runAllBenchmarks() {
        std::cout << "\n=== Canvas Performance Benchmark ===\n" << std::endl;
        
        // Create test canvases
        auto cpuCanvas = std::make_unique<BenchmarkCanvas>();
        auto gpuCanvas = std::make_unique<BenchmarkGLCanvas>();
        
        // Set canvas sizes
        const int canvasWidth = 800;
        const int canvasHeight = 600;
        cpuCanvas->resize(canvasWidth, canvasHeight);
        gpuCanvas->resize(canvasWidth, canvasHeight);
        
        // Show canvases to ensure proper initialization
        cpuCanvas->show();
        gpuCanvas->show();
        QApplication::processEvents();
        
        // Wait for OpenGL initialization
        QThread::msleep(100);
        QApplication::processEvents();
        
        // Run benchmarks
        std::cout << "1. Empty Canvas Rendering Benchmark" << std::endl;
        benchmarkEmptyCanvas(cpuCanvas.get(), gpuCanvas.get());
        
        std::cout << "\n2. Canvas with Elements Benchmark" << std::endl;
        benchmarkCanvasWithElements(cpuCanvas.get(), gpuCanvas.get());
        
        std::cout << "\n3. Simulated Drawing Operations Benchmark" << std::endl;
        benchmarkDrawingOperations();
        
        std::cout << "\n4. Mouse Event Processing Benchmark" << std::endl;
        benchmarkMouseEvents(cpuCanvas.get(), gpuCanvas.get());
        
        std::cout << "\n5. Canvas Update Performance" << std::endl;
        benchmarkCanvasUpdates(cpuCanvas.get(), gpuCanvas.get());
    }

private:
    static BenchmarkResult measurePerformance(
        const QString& name,
        std::function<void()> setup,
        std::function<void()> operation,
        std::function<void()> cleanup,
        int iterations = 100
    ) {
        std::vector<double> times;
        times.reserve(iterations);
        
        // Warmup
        for (int i = 0; i < 5; ++i) {
            setup();
            operation();
            cleanup();
        }
        
        // Actual measurements
        QElapsedTimer timer;
        for (int i = 0; i < iterations; ++i) {
            setup();
            
            timer.start();
            operation();
            double elapsed = timer.nsecsElapsed() / 1000000.0; // Convert to ms
            
            cleanup();
            times.push_back(elapsed);
            
            QApplication::processEvents();
        }
        
        // Calculate statistics
        std::sort(times.begin(), times.end());
        double min = times.front();
        double max = times.back();
        double avg = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        double median = times[times.size() / 2];
        
        return {name, min, max, avg, median, iterations};
    }
    
    static void printResult(const BenchmarkResult& result) {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "  " << result.name.toStdString() << ":" << std::endl;
        std::cout << "    Min: " << result.minTime << " ms" << std::endl;
        std::cout << "    Max: " << result.maxTime << " ms" << std::endl;
        std::cout << "    Avg: " << result.avgTime << " ms" << std::endl;
        std::cout << "    Median: " << result.medianTime << " ms" << std::endl;
    }
    
    static void benchmarkEmptyCanvas(BenchmarkCanvas* cpuCanvas, BenchmarkGLCanvas* gpuCanvas) {
        // CPU Canvas empty rendering
        auto cpuResult = measurePerformance(
            "CPU Canvas - Empty",
            []() {},
            [&]() { cpuCanvas->benchmarkPaint(); },
            []() {}
        );
        
        // GPU Canvas empty rendering
        auto gpuResult = measurePerformance(
            "GPU Canvas - Empty",
            []() {},
            [&]() { gpuCanvas->benchmarkPaint(); },
            []() {}
        );
        
        printResult(cpuResult);
        printResult(gpuResult);
    }
    
    static void benchmarkCanvasWithElements(BenchmarkCanvas* cpuCanvas, BenchmarkGLCanvas* gpuCanvas) {
        // Test with 9 elements first
        std::vector<QPoint> smallFramePositions = {
            QPoint(50, 50), QPoint(200, 50), QPoint(350, 50),
            QPoint(50, 200), QPoint(200, 200), QPoint(350, 200),
            QPoint(50, 350), QPoint(200, 350), QPoint(350, 350)
        };
        
        // Create frames on CPU canvas
        for (const auto& pos : smallFramePositions) {
            cpuCanvas->simulateMousePress(pos);
        }
        
        auto cpuWithSmallFrames = measurePerformance(
            "CPU Canvas - With 9 Frames",
            []() {},
            [&]() { cpuCanvas->benchmarkPaint(); },
            []() {},
            50
        );
        
        // Clear and create frames on GPU canvas
        gpuCanvas->hide();
        gpuCanvas->show();
        QApplication::processEvents();
        
        for (const auto& pos : smallFramePositions) {
            gpuCanvas->simulateMousePress(pos);
        }
        
        auto gpuWithSmallFrames = measurePerformance(
            "GPU Canvas - With 9 Frames",
            []() {},
            [&]() { gpuCanvas->benchmarkPaint(); },
            []() {},
            50
        );
        
        printResult(cpuWithSmallFrames);
        printResult(gpuWithSmallFrames);
        
        // Now test with 300 elements
        std::cout << "\n  Testing with 300 elements..." << std::endl;
        
        // Clear canvases
        cpuCanvas->hide();
        cpuCanvas->show();
        gpuCanvas->hide();
        gpuCanvas->show();
        QApplication::processEvents();
        QThread::msleep(100);
        
        // Generate 300 positions in a grid pattern
        std::vector<QPoint> largeFramePositions;
        int gridSize = 20; // 20x15 grid = 300 elements
        int spacing = 40;
        for (int row = 0; row < 15; ++row) {
            for (int col = 0; col < gridSize; ++col) {
                largeFramePositions.push_back(QPoint(10 + col * spacing, 10 + row * spacing));
            }
        }
        
        // Create 300 frames on CPU canvas
        std::cout << "  Creating 300 frames on CPU Canvas..." << std::endl;
        for (const auto& pos : largeFramePositions) {
            cpuCanvas->simulateMousePress(pos);
            if (largeFramePositions.size() > 100 && 
                (&pos - &largeFramePositions[0]) % 50 == 0) {
                QApplication::processEvents(); // Process events periodically
            }
        }
        
        auto cpuWithLargeFrames = measurePerformance(
            "CPU Canvas - With 300 Frames",
            []() {},
            [&]() { cpuCanvas->benchmarkPaint(); },
            []() {},
            20  // Fewer iterations due to higher complexity
        );
        
        // Clear and create 300 frames on GPU canvas
        gpuCanvas->hide();
        gpuCanvas->show();
        QApplication::processEvents();
        QThread::msleep(100);
        
        std::cout << "  Creating 300 frames on GPU Canvas..." << std::endl;
        for (const auto& pos : largeFramePositions) {
            gpuCanvas->simulateMousePress(pos);
            if (largeFramePositions.size() > 100 && 
                (&pos - &largeFramePositions[0]) % 50 == 0) {
                QApplication::processEvents(); // Process events periodically
            }
        }
        
        auto gpuWithLargeFrames = measurePerformance(
            "GPU Canvas - With 300 Frames",
            []() {},
            [&]() { gpuCanvas->benchmarkPaint(); },
            []() {},
            20  // Fewer iterations due to higher complexity
        );
        
        printResult(cpuWithLargeFrames);
        printResult(gpuWithLargeFrames);
        
        // Calculate speedup/slowdown for 300 frames
        if (gpuWithLargeFrames.avgTime < cpuWithLargeFrames.avgTime) {
            double speedup = cpuWithLargeFrames.avgTime / gpuWithLargeFrames.avgTime;
            std::cout << "  GPU Performance with 300 frames: " << std::fixed << std::setprecision(2) 
                      << speedup << "x faster than CPU" << std::endl;
        } else {
            double slowdown = gpuWithLargeFrames.avgTime / cpuWithLargeFrames.avgTime;
            std::cout << "  GPU Performance with 300 frames: " << std::fixed << std::setprecision(2) 
                      << slowdown << "x slower than CPU" << std::endl;
        }
    }
    
    static void benchmarkDrawingOperations() {
        // Benchmark raw drawing operations that would happen in a canvas
        QPixmap pixmap(800, 600);
        
        // CPU rendering simulation
        auto cpuDrawing = measurePerformance(
            "CPU Drawing - 100 Rectangles",
            []() {},
            [&]() {
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                
                for (int i = 0; i < 100; ++i) {
                    int x = (i * 73) % 700;
                    int y = ((i / 10) * 50) % 500;
                    painter.fillRect(x, y, 50, 50, QColor(i % 256, (i * 2) % 256, (i * 3) % 256));
                    painter.drawRect(x, y, 50, 50);
                }
            },
            []() {}
        );
        
        auto cpuText = measurePerformance(
            "CPU Drawing - 50 Text Items",
            []() {},
            [&]() {
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                QFont font("Arial", 12);
                painter.setFont(font);
                
                for (int i = 0; i < 50; ++i) {
                    int x = (i * 100) % 700;
                    int y = ((i / 7) * 70) % 500;
                    painter.drawText(x, y, QString("Text %1").arg(i));
                }
            },
            []() {}
        );
        
        printResult(cpuDrawing);
        printResult(cpuText);
    }
    
    static void benchmarkMouseEvents(BenchmarkCanvas* cpuCanvas, BenchmarkGLCanvas* gpuCanvas) {
        // Benchmark mouse event processing
        auto cpuMouseEvents = measurePerformance(
            "CPU Canvas - Mouse Event Processing",
            []() {},
            [&]() {
                for (int i = 0; i < 10; ++i) {
                    QPoint pos(100 + i * 10, 100 + i * 10);
                    cpuCanvas->simulateMouseMove(pos);
                }
            },
            []() {}
        );
        
        auto gpuMouseEvents = measurePerformance(
            "GPU Canvas - Mouse Event Processing",
            []() {},
            [&]() {
                for (int i = 0; i < 10; ++i) {
                    QPoint pos(100 + i * 10, 100 + i * 10);
                    gpuCanvas->simulateMouseMove(pos);
                }
            },
            []() {}
        );
        
        printResult(cpuMouseEvents);
        printResult(gpuMouseEvents);
    }
    
    static void benchmarkCanvasUpdates(BenchmarkCanvas* cpuCanvas, BenchmarkGLCanvas* gpuCanvas) {
        // Measure update/refresh performance
        const int updateCount = 60; // Simulate 60 updates
        
        QElapsedTimer cpuTimer;
        cpuTimer.start();
        for (int i = 0; i < updateCount; ++i) {
            cpuCanvas->update();
            QApplication::processEvents();
        }
        double cpuTime = cpuTimer.elapsed();
        double cpuFPS = updateCount / (cpuTime / 1000.0);
        
        QElapsedTimer gpuTimer;
        gpuTimer.start();
        for (int i = 0; i < updateCount; ++i) {
            gpuCanvas->update();
            QApplication::processEvents();
        }
        double gpuTime = gpuTimer.elapsed();
        double gpuFPS = updateCount / (gpuTime / 1000.0);
        
        std::cout << "  CPU Canvas - Update Rate: " << std::fixed << std::setprecision(1) 
                  << cpuFPS << " fps (Total time: " << cpuTime << " ms)" << std::endl;
        std::cout << "  GPU Canvas - Update Rate: " << std::fixed << std::setprecision(1) 
                  << gpuFPS << " fps (Total time: " << gpuTime << " ms)" << std::endl;
        
        // Performance comparison
        if (gpuTime < cpuTime) {
            double speedup = cpuTime / gpuTime;
            std::cout << "\n  GPU Performance: " << std::fixed << std::setprecision(2) 
                      << speedup << "x faster than CPU" << std::endl;
        } else {
            double slowdown = gpuTime / cpuTime;
            std::cout << "\n  GPU Performance: " << std::fixed << std::setprecision(2) 
                      << slowdown << "x slower than CPU" << std::endl;
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    QCoreApplication::setApplicationName("Canvas Performance Benchmark");
    QCoreApplication::setOrganizationName("CubitCpp");
    
    // Run benchmarks
    BenchmarkRunner::runAllBenchmarks();
    
    std::cout << "\nBenchmark completed!" << std::endl;
    
    return 0;
}