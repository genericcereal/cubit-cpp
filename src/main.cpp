#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngineQuick>
#include <QDebug>

#include "Element.h"
#include "CanvasElement.h"
#include "DesignElement.h"
#include "ScriptElement.h"
#include "Frame.h"
#include "Text.h"
#include "platforms/web/WebTextInput.h"
#include "Variable.h"
#include "Component.h"
#include "FrameComponentInstance.h"
#include "FrameComponentVariant.h"
#include "TextComponentVariant.h"
#include "TextComponentInstance.h"
#include "Node.h"
#include "Edge.h"
#include "CanvasController.h"
#include "DesignCanvas.h"
#include "ElementModel.h"
#include "SelectionManager.h"
#include "ViewportCache.h"
#include "ConsoleMessageRepository.h"
#include "Application.h"
#include "Project.h"
#include "Panels.h"
#include "Scripts.h"
#include "ScriptCompiler.h"
#include "ElementFilterProxy.h"
#include "PrototypeController.h"
#include "DesignControlsController.h"
#include "AuthenticationManager.h"
#include "UrlSchemeHandler.h"


int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();

    // Enable high DPI scaling for better rendering quality
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Enable antialiasing for smoother rendering
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(8); // Increase to 8x MSAA
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("Cubit");
    app.setOrganizationName("Cubit");
    
    // Create authentication manager
    AuthenticationManager* authManager = new AuthenticationManager(&app);
    
    // Create and register URL scheme handler
    UrlSchemeHandler* urlHandler = new UrlSchemeHandler(authManager, &app);
    urlHandler->registerUrlScheme();
    
    // Handle command line arguments for URL scheme on some platforms
    QStringList args = app.arguments();
    if (args.size() > 1) {
        for (int i = 1; i < args.size(); ++i) {
            QUrl url(args[i]);
            if (url.scheme() == "myapp") {
                urlHandler->handleUrl(url);
            }
        }
    }


    QQmlApplicationEngine engine;

    // Register C++ types with QML
    qmlRegisterUncreatableType<Element>("Cubit", 1, 0, "Element", "Element is an abstract base class");
    qmlRegisterUncreatableType<CanvasElement>("Cubit", 1, 0, "CanvasElement", "CanvasElement is an abstract base class");
    qmlRegisterUncreatableType<DesignElement>("Cubit", 1, 0, "DesignElement", "DesignElement is an abstract base class");
    qmlRegisterUncreatableType<ScriptElement>("Cubit", 1, 0, "ScriptElement", "ScriptElement is an abstract base class");
    qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
    qmlRegisterType<Text>("Cubit", 1, 0, "TextElement");  // Rename to avoid conflict with QML Text
    qmlRegisterType<WebTextInput>("Cubit", 1, 0, "WebTextInput");
    qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
    qmlRegisterType<Component>("Cubit", 1, 0, "ComponentElement");
    qmlRegisterType<FrameComponentInstance>("Cubit", 1, 0, "FrameComponentInstance");
    qmlRegisterType<FrameComponentVariant>("Cubit", 1, 0, "FrameComponentVariant");
    qmlRegisterType<TextComponentVariant>("Cubit", 1, 0, "TextComponentVariant");
    qmlRegisterType<TextComponentInstance>("Cubit", 1, 0, "TextComponentInstance");
    qmlRegisterType<Node>("Cubit", 1, 0, "Node");
    qmlRegisterType<Edge>("Cubit", 1, 0, "Edge");
    
    // Register singleton controllers
    qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
    qmlRegisterType<DesignCanvas>("Cubit", 1, 0, "DesignCanvas");
    qmlRegisterType<ElementModel>("Cubit", 1, 0, "ElementModel");
    qmlRegisterType<SelectionManager>("Cubit", 1, 0, "SelectionManager");
    qmlRegisterType<ViewportCache>("Cubit", 1, 0, "ViewportCache");
    qmlRegisterType<Project>("Cubit", 1, 0, "CanvasData");
    qmlRegisterType<Panels>("Cubit", 1, 0, "Panels");
    qmlRegisterType<Scripts>("Cubit", 1, 0, "Scripts");
    qmlRegisterType<ScriptCompiler>("Cubit", 1, 0, "ScriptCompiler");
    qmlRegisterType<ElementFilterProxy>("Cubit", 1, 0, "ElementFilterProxy");
    qmlRegisterType<PrototypeController>("Cubit", 1, 0, "PrototypeController");
    
    // Register authentication types
    qmlRegisterType<AuthenticationManager>("Cubit", 1, 0, "AuthenticationManager");
    
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
    

    // Create and register DesignControlsController as context property
    DesignControlsController* designControlsController = new DesignControlsController(appInstance, &app);
    engine.rootContext()->setContextProperty("designControls", designControlsController);
    
    // Register authentication manager as context property
    engine.rootContext()->setContextProperty("authManager", authManager);

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