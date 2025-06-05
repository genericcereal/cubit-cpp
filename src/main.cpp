#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "DetailPanel.h"
#include "Canvas.h"  // 🔹 include your new component
#include "ElementList.h"

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

    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    Canvas *canvas = new Canvas();  // 🔹 use your component
    splitter->addWidget(canvas);

    DetailPanel *detailPanel = new DetailPanel();
    splitter->addWidget(detailPanel);
    
    // Connect canvas element creation to element list
    QObject::connect(canvas, &Canvas::elementCreated, 
                     detailPanel->getElementList(), &ElementList::addElement);

    // Configure layout behavior
    splitter->setStretchFactor(0, 1); // Left (main content) stretches
    splitter->setStretchFactor(1, 0); // Right (detail panel) stays fixed
    splitter->setSizes({600, 300});   // Set initial sizes


    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(splitter);

    window.show();
    return app.exec();
}