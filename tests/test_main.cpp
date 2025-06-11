#include <QTest>
#include <QCoreApplication>
#include <QQmlEngine>
#include <QQmlComponent>

// Include the C++ types we need to register for testing
#include "../src/Element.h"
#include "../src/Frame.h"
#include "../src/Text.h"
#include "../src/Html.h"
#include "../src/Variable.h"
#include "../src/CanvasController.h"
#include "../src/ElementModel.h"
#include "../src/SelectionManager.h"

// Register types once for all tests
static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
        qmlRegisterType<Element>("Cubit", 1, 0, "Element");
        qmlRegisterType<Frame>("Cubit", 1, 0, "Frame");
        qmlRegisterType<Text>("Cubit", 1, 0, "Text");
        qmlRegisterType<Html>("Cubit", 1, 0, "Html");
        qmlRegisterType<Variable>("Cubit", 1, 0, "Variable");
        qmlRegisterType<CanvasController>("Cubit", 1, 0, "CanvasController");
        qmlRegisterType<ElementModel>("Cubit", 1, 0, "ElementModel");
        qmlRegisterType<SelectionManager>("Cubit", 1, 0, "SelectionManager");
        registered = true;
    }
}

// Include the test class implementations
#include "tst_FrameOperations.cpp"
#include "tst_SelectionManager.cpp"
#include "tst_ElementModel.cpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    
    // Register QML types
    registerTypes();
    
    int status = 0;
    
    // Run each test suite
    {
        tst_FrameOperations tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    
    {
        tst_SelectionManager tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    
    {
        tst_ElementModel tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    
    return status;
}