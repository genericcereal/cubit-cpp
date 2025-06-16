TEMPLATE = app
TARGET = cubit-tests
QT += quick qml testlib webenginecore webenginequick
CONFIG += c++17 qt warn_on testcase

# Include paths
INCLUDEPATH += ../src

# Test sources
SOURCES += \
    test_main.cpp \
    tst_FrameOperations.cpp \
    tst_SelectionManager.cpp \
    tst_ElementModel.cpp \
    tst_ScriptCompiler.cpp

# Include the main project's source files directly
SOURCES += \
    ../src/Element.cpp \
    ../src/CanvasElement.cpp \
    ../src/DesignElement.cpp \
    ../src/ScriptElement.cpp \
    ../src/Frame.cpp \
    ../src/Text.cpp \
    ../src/Html.cpp \
    ../src/Variable.cpp \
    ../src/Node.cpp \
    ../src/Edge.cpp \
    ../src/CanvasController.cpp \
    ../src/DragManager.cpp \
    ../src/CreationManager.cpp \
    ../src/HitTestService.cpp \
    ../src/JsonImporter.cpp \
    ../src/QuadTree.cpp \
    ../src/ElementModel.cpp \
    ../src/SelectionManager.cpp \
    ../src/Scripts.cpp \
    ../src/ScriptCompiler.cpp

# Headers
HEADERS += \
    QmlTestHelper.h \
    ../src/Element.h \
    ../src/CanvasElement.h \
    ../src/DesignElement.h \
    ../src/ScriptElement.h \
    ../src/Frame.h \
    ../src/Text.h \
    ../src/Html.h \
    ../src/Variable.h \
    ../src/Node.h \
    ../src/Edge.h \
    ../src/CanvasController.h \
    ../src/DragManager.h \
    ../src/CreationManager.h \
    ../src/HitTestService.h \
    ../src/JsonImporter.h \
    ../src/QuadTree.h \
    ../src/ElementModel.h \
    ../src/SelectionManager.h \
    ../src/Config.h \
    ../src/Scripts.h \
    ../src/ScriptCompiler.h \
    ../src/UniqueIdGenerator.h \
    ../src/HandleType.h

# Import path for QML modules
QML_IMPORT_PATH = ../qml

# Additional import path for finding our custom QML types
QML_IMPORT_PATH += ..