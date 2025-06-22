#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngineQuick>

#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "ScriptElement.h"
#include "Frame.h"
#include "Text.h"
#include "Html.h"
#include "Variable.h"
#include "Component.h"
#include "ComponentInstance.h"
#include "ComponentVariant.h"
#include "Node.h"
#include "Edge.h"
#include "CanvasController.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ViewportCache.h"
#include "ConsoleMessageRepository.h"
#include "Application.h"
#include "Project.h"
#include "Panels.h"
#include "Scripts.h"
#include "ScriptCompiler.h"

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
    qmlRegisterUncreatableType<CanvasElement>("Cubit", 1, 0, "CanvasElement", "CanvasElement is an abstract base class");
    qmlRegisterUncreatableType<DesignElement>("Cubit", 1, 0, "DesignElement", "DesignElement is an abstract base class");
    qmlRegisterUncreatableType<ScriptElement>("Cubit", 1, 0, "ScriptElement", "ScriptElement is an abstract base class");
    qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
    qmlRegisterType<Text>("Cubit", 1, 0, "TextElement");  // Rename to avoid conflict with QML Text
    qmlRegisterType<Html>("Cubit", 1, 0, "Html");
    qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
    qmlRegisterType<Component>("Cubit", 1, 0, "ComponentElement");
    qmlRegisterType<ComponentInstance>("Cubit", 1, 0, "ComponentInstance");
    qmlRegisterType<ComponentVariant>("Cubit", 1, 0, "ComponentVariant");
    qmlRegisterType<Node>("Cubit", 1, 0, "Node");
    qmlRegisterType<Edge>("Cubit", 1, 0, "Edge");
    
    // Register singleton controllers
    qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
    qmlRegisterType<ElementModel>("Cubit", 1, 0, "ElementModel");
    qmlRegisterType<SelectionManager>("Cubit", 1, 0, "SelectionManager");
    qmlRegisterType<ViewportCache>("Cubit", 1, 0, "ViewportCache");
    qmlRegisterType<Project>("Cubit", 1, 0, "CanvasData");
    qmlRegisterType<Panels>("Cubit", 1, 0, "Panels");
    qmlRegisterType<Scripts>("Cubit", 1, 0, "Scripts");
    qmlRegisterType<ScriptCompiler>("Cubit", 1, 0, "ScriptCompiler");
    
    // Register singleton ConsoleMessageRepository
    qmlRegisterSingletonType<ConsoleMessageRepository>("Cubit", 1, 0, "ConsoleMessageRepository",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return ConsoleMessageRepository::instance();
        });

    // Register Application singleton
    Application* appInstance = new Application(&app);
    qmlRegisterSingletonType<Application>("Cubit", 1, 0, "Application",
        [appInstance](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return appInstance;
        });

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