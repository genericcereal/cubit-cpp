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
    tst_ElementModel.cpp

# Include the main project's source files directly
SOURCES += \
    ../src/Element.cpp \
    ../src/Frame.cpp \
    ../src/Text.cpp \
    ../src/Html.cpp \
    ../src/Variable.cpp \
    ../src/CanvasController.cpp \
    ../src/ElementModel.cpp \
    ../src/SelectionManager.cpp

# Headers
HEADERS += \
    QmlTestHelper.h \
    ../src/Element.h \
    ../src/Frame.h \
    ../src/Text.h \
    ../src/Html.h \
    ../src/Variable.h \
    ../src/CanvasController.h \
    ../src/ElementModel.h \
    ../src/SelectionManager.h \
    ../src/Config.h

# Import path for QML modules
QML_IMPORT_PATH = ../qml

# Additional import path for finding our custom QML types
QML_IMPORT_PATH += ..