#include <QtQuickTest>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngineQuick>

#include "../src/Element.h"
#include "../src/Frame.h"
#include "../src/Text.h"
#include "../src/Html.h"
#include "../src/Variable.h"
#include "../src/CanvasController.h"
#include "../src/ElementModel.h"
#include "../src/SelectionManager.h"

// Custom setup function to register types
class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        // Register C++ types with QML (same as in main.cpp)
        qmlRegisterUncreatableType<Element>("Cubit", 1, 0, "Element", "Element is an abstract base class");
        qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
        qmlRegisterType<Text>("Cubit", 1, 0, "TextElement");
        qmlRegisterType<Html>("Cubit", 1, 0, "Html");
        qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
        
        qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
        qmlRegisterType<ElementModel>("Cubit", 1, 0, "ElementModel");
        qmlRegisterType<SelectionManager>("Cubit", 1, 0, "SelectionManager");

        // Register QML singleton
        qmlRegisterSingletonType(QUrl("qrc:/qml/Config.qml"), "Cubit.UI", 1, 0, "Config");
    }
};

int main(int argc, char *argv[])
{
    // Initialize QtWebEngine
    QtWebEngineQuick::initialize();

    // Enable antialiasing
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    // Set up and run tests
    Setup setup;
    return quick_test_main_with_setup(argc, argv, "CubitTests", nullptr, &setup);
}

#include "test_main.moc"