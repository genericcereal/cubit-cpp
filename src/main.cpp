#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngineQuick>

#include "Element.h"
#include "Frame.h"
#include "Text.h"
#include "Html.h"
#include "Variable.h"
#include "Node.h"
#include "Edge.h"
#include "CanvasController.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ViewportCache.h"

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();

    // Enable antialiasing for smoother rendering
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4); // 4x MSAA
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);
    app.setApplicationName("Cubit");
    app.setOrganizationName("Cubit");

    QQmlApplicationEngine engine;

    // Register C++ types with QML
    qmlRegisterUncreatableType<Element>("Cubit", 1, 0, "Element", "Element is an abstract base class");
    qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
    qmlRegisterType<Text>("Cubit", 1, 0, "TextElement");  // Rename to avoid conflict with QML Text
    qmlRegisterType<Html>("Cubit", 1, 0, "Html");
    qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
    qmlRegisterType<Node>("Cubit", 1, 0, "Node");
    qmlRegisterType<Edge>("Cubit", 1, 0, "Edge");
    
    // Register singleton controllers
    qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
    qmlRegisterType<ElementModel>("Cubit", 1, 0, "ElementModel");
    qmlRegisterType<SelectionManager>("Cubit", 1, 0, "SelectionManager");
    qmlRegisterType<ViewportCache>("Cubit", 1, 0, "ViewportCache");

    // Register QML singleton
    qmlRegisterSingletonType(QUrl("qrc:/qml/Config.qml"), "Cubit.UI", 1, 0, "Config");

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}