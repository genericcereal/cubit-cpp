#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QEvent>
#include <QDebug>
#include <functional>

#include "DetailPanel.h"
#include "Canvas.h"
#include "ElementList.h"
#include "ActionsPanel.h"
#include "FPSWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set global application styles
    app.setStyleSheet(R"(
        QFrame[panelStyle="true"] {
            background-color: rgba(50, 50, 50, 0.9);
        }
    )");

    QWidget window;
    window.setWindowTitle("Resizable Detail Panel");
    window.resize(900, 600);

    // Create a central widget to hold the canvas and overlays
    QWidget *centralWidget = new QWidget();
    
    // Create layout for central widget to properly size canvas
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create canvas
    Canvas *canvas = new Canvas();
    centralLayout->addWidget(canvas);
    
    // Create ActionsPanel as overlay on centralWidget (not canvas)
    ActionsPanel *actionsPanel = new ActionsPanel(centralWidget);
    actionsPanel->setCanvas(canvas);
    actionsPanel->raise();  // Ensure it's on top
    
    // Create FPSWidget as overlay on centralWidget (not canvas)
    FPSWidget *fpsWidget = new FPSWidget(centralWidget);
    fpsWidget->setFixedSize(100, 60);
    fpsWidget->start();
    fpsWidget->raise();  // Ensure it's on top
    
    // Set rendering type in FPS widget
    fpsWidget->setRenderingType(canvas->getRenderingType());
    
    // Connect canvas mode changes to ActionsPanel
    QObject::connect(canvas, &Canvas::modeChanged,
                     [actionsPanel](const QString &mode) {
                         actionsPanel->setModeSelection(mode);
                     });
    
    // Connect overlaysNeedRaise signal to raise overlay panels
    QObject::connect(canvas, &Canvas::overlaysNeedRaise,
                     [actionsPanel, fpsWidget]() {
                         actionsPanel->raise();
                         fpsWidget->raise();
                     });

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(centralWidget);

    DetailPanel *detailPanel = new DetailPanel();
    splitter->addWidget(detailPanel);
    
    // Connect canvas element creation to element list
    QObject::connect(canvas, &Canvas::elementCreated, 
                     detailPanel->getElementList(), &ElementList::addElement);

    // Configure layout behavior
    splitter->setStretchFactor(0, 1); // Left (main content) stretches
    splitter->setStretchFactor(1, 0); // Right (detail panel) stays fixed
    splitter->setSizes({600, 300});   // Set initial sizes
    
    // Function to position overlays
    auto positionOverlays = [centralWidget, canvas, actionsPanel, fpsWidget]() {
        // Position ActionsPanel at bottom center of centralWidget
        int panelWidth = actionsPanel->width();
        int panelHeight = actionsPanel->height();
        int x = (centralWidget->width() - panelWidth) / 2;
        int y = centralWidget->height() - panelHeight - 10;
        actionsPanel->move(x, y);
        
        // Position FPSWidget at top right of centralWidget
        int fpsX = centralWidget->width() - fpsWidget->width() - 10;
        int fpsY = 10;
        fpsWidget->move(fpsX, fpsY);
    };
    
    // Install event filter to catch centralWidget resize events
    class ResizeFilter : public QObject {
        std::function<void()> callback;
    public:
        ResizeFilter(std::function<void()> cb) : callback(cb) {}
        bool eventFilter(QObject *, QEvent *event) override {
            if (event->type() == QEvent::Resize) {
                callback();
            }
            return false;
        }
    };
    
    auto *resizeFilter = new ResizeFilter(positionOverlays);
    centralWidget->installEventFilter(resizeFilter);
    
    // Also reposition when splitter moves
    QObject::connect(splitter, &QSplitter::splitterMoved,
                     [positionOverlays]() {
                         positionOverlays();
                     });

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(splitter);

    window.show();
    
    // Initial positioning
    positionOverlays();
    return app.exec();
}