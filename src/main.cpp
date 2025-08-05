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
#include "Shape.h"
#include "Variable.h"
#include "BoxShadow.h"
#include "Node.h"
#include "Edge.h"
#include "Component.h"
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
#include "ShapeControlsController.h"
#include "AuthenticationManager.h"
#include "UrlSchemeHandler.h"
#include "ElementTypeRegistry.h"
#include "PlatformConfig.h"
#include "CanvasContext.h"
#include "PropertyRegistry.h"
#include "PropertyTypeMapper.h"
#include "AdaptiveThrottler.h"
#include "ConfigObject.h"
#include "VariableBinding.h"
#include "GoogleFonts.h"

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();

    // Enable high DPI scaling for better rendering quality
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Enable antialiasing for smoother rendering
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4); // Use 4x MSAA which is widely supported
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("Cubit");
    app.setOrganizationName("Cubit");
    
    // Quit when the last window is closed
    app.setQuitOnLastWindowClosed(true);

    // Ensure native menu bar on macOS
#ifdef Q_OS_MAC
    app.setAttribute(Qt::AA_DontUseNativeMenuBar, false);
#endif

    // Initialize the ElementTypeRegistry with default types
    ElementTypeRegistry::instance().initializeDefaultTypes();
    
    // Load property type mappings
    PropertyTypeMapper::instance()->loadMappings(":/data/src/property-types.json");

    // Create authentication manager
    AuthenticationManager *authManager = new AuthenticationManager(&app);

    // Create and register URL scheme handler
    UrlSchemeHandler *urlHandler = new UrlSchemeHandler(authManager, &app);
    urlHandler->registerUrlScheme();

    // If user is already authenticated (has saved tokens), refresh them on startup
    if (authManager->isAuthenticated())
    {
        authManager->refreshAccessToken();
    }

    // Handle command line arguments for URL scheme on some platforms
    QStringList args = app.arguments();
    if (args.size() > 1)
    {
        for (int i = 1; i < args.size(); ++i)
        {
            QUrl url(args[i]);
            if (url.scheme() == "cubitapp")
            {
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
    qmlRegisterType<Text>("Cubit", 1, 0, "TextElement"); // Rename to avoid conflict with QML Text
    qmlRegisterType<WebTextInput>("Cubit", 1, 0, "WebTextInput");
    qmlRegisterType<Shape>("Cubit", 1, 0, "ShapeElement");
    qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
    qmlRegisterType<Node>("Cubit", 1, 0, "Node");
    qmlRegisterType<Edge>("Cubit", 1, 0, "Edge");
    qmlRegisterType<ComponentElement>("Cubit", 1, 0, "ComponentElement");
    
    // Register value types
    qRegisterMetaType<BoxShadow>("BoxShadow");
    qmlRegisterUncreatableMetaObject(BoxShadow::staticMetaObject, "Cubit", 1, 0, "BoxShadow", "BoxShadow is a value type");

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
    qmlRegisterType<PlatformConfig>("Cubit", 1, 0, "PlatformConfig");
    qmlRegisterType<ElementFilterProxy>("Cubit", 1, 0, "ElementFilterProxy");
    qmlRegisterType<PrototypeController>("Cubit", 1, 0, "PrototypeController");
    qmlRegisterType<DesignControlsController>("Cubit", 1, 0, "DesignControlsController");
    qmlRegisterType<ShapeControlsController>("Cubit", 1, 0, "ShapeControlsController");
    
    // Register throttling types
    qmlRegisterType<AdaptiveThrottler>("Cubit", 1, 0, "AdaptiveThrottler");
    
    // Register variable binding types
    qmlRegisterType<VariableBinding>("Cubit", 1, 0, "VariableBinding");
    qmlRegisterType<VariableBindingManager>("Cubit", 1, 0, "VariableBindingManager");
    
    // Register CanvasContext as uncreatable base class
    qmlRegisterUncreatableType<CanvasContext>("Cubit", 1, 0, "CanvasContext", "CanvasContext is an abstract base class");

    // Register authentication types
    qmlRegisterType<AuthenticationManager>("Cubit", 1, 0, "AuthenticationManager");

    // Register ConsoleMessageRepository as a regular type (not singleton)
    qmlRegisterType<ConsoleMessageRepository>("Cubit", 1, 0, "ConsoleMessageRepository");
    
    // Register GoogleFonts singleton
    qmlRegisterSingletonType<GoogleFonts>("Cubit", 1, 0, "GoogleFonts",
                                         [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject *
                                         {
                                             Q_UNUSED(engine)
                                             Q_UNUSED(scriptEngine)
                                             return GoogleFonts::instance();
                                         });
    
    // Register PropertyRegistry
    qmlRegisterType<PropertyRegistry>("Cubit", 1, 0, "PropertyRegistry");
    qmlRegisterType<PropertyMetadata>("Cubit", 1, 0, "PropertyMetadata");

    // Register Application singleton
    Application *appInstance = new Application(&app);
    appInstance->setAuthenticationManager(authManager);
    appInstance->setEngine(&engine);
    
    // Setup single instance handling
    const QString uniqueKey = "cubit-app-instance";
    if (!appInstance->setupSingleInstance(uniqueKey)) {
        // Another instance is already running
        // Send command line arguments to the running instance
        if (args.size() > 1) {
            for (int i = 1; i < args.size(); ++i) {
                appInstance->sendMessageToRunningInstance(args[i]);
            }
        }
        return 0; // Exit this instance
    }
    
    // Connect to handle messages from other instances
    QObject::connect(appInstance, &Application::messageReceivedFromOtherInstance,
                     [urlHandler](const QString& message) {
        QUrl url(message);
        if (url.scheme() == "cubitapp") {
            urlHandler->handleUrl(url);
        }
    });


    // IMPORTANT: Set Application as context property FIRST
    // This ensures it's available immediately when QML loads
    engine.rootContext()->setContextProperty("Application", appInstance);

    // Force the engine to update its context
    engine.rootContext()->setContextObject(nullptr);

    // Also register as singleton type for consistency, but context property takes precedence
    qmlRegisterSingletonType<Application>("Cubit", 1, 0, "Application",
                                          [appInstance](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject *
                                          {
                                              Q_UNUSED(engine)
                                              Q_UNUSED(scriptEngine)
                                              return appInstance;
                                          });

    // No longer creating global DesignControlsController - each window creates its own

    // Register authentication manager as context property
    engine.rootContext()->setContextProperty("authManager", authManager);

    // Register ConfigObject singleton
    qmlRegisterSingletonType<ConfigObject>("Cubit", 1, 0, "ConfigObject",
                                          [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject *
                                          {
                                              Q_UNUSED(engine)
                                              Q_UNUSED(scriptEngine)
                                              return new ConfigObject();
                                          });
    
    // Register PropertyTypeMapper singleton
    qmlRegisterSingletonType<PropertyTypeMapper>("Cubit", 1, 0, "PropertyTypeMapper",
                                                [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject *
                                                {
                                                    Q_UNUSED(engine)
                                                    Q_UNUSED(scriptEngine)
                                                    return PropertyTypeMapper::instance();
                                                });

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl)
                     {
        if (!obj && url == objUrl) {
            qCritical() << "Failed to load main.qml";
            QCoreApplication::exit(-1);
        } else if (obj && url == objUrl) {
            // Main QML loaded successfully
        } }, Qt::QueuedConnection);

    // Load QML immediately since context properties are already set
    engine.load(url);

    int result = app.exec();
    
    // IMPORTANT: Clear all QML connections before Application is destroyed
    // This prevents QML from trying to access destroyed C++ objects during shutdown
    QQmlContext* rootContext = engine.rootContext();
    if (rootContext) {
        rootContext->setContextProperty("Application", nullptr);
    }
    
    return result;
}