TEMPLATE = app
TARGET = cubit-tests
QT += quick qml testlib quickcontrols2 webenginecore webenginequick

CONFIG += qt warn_on qmltestcase

# Include paths
INCLUDEPATH += ../src

# Source files from main project
SOURCES += \
    ../src/Element.cpp \
    ../src/Frame.cpp \
    ../src/Text.cpp \
    ../src/Html.cpp \
    ../src/Variable.cpp \
    ../src/CanvasController.cpp \
    ../src/ElementModel.cpp \
    ../src/SelectionManager.cpp \
    test_main.cpp

# Header files from main project
HEADERS += \
    ../src/Element.h \
    ../src/Frame.h \
    ../src/Text.h \
    ../src/Html.h \
    ../src/Variable.h \
    ../src/CanvasController.h \
    ../src/ElementModel.h \
    ../src/SelectionManager.h \
    ../src/Config.h

# QML test files
QML_FILES += \
    tst_FrameOperations.qml

# Resources from main project
RESOURCES += \
    ../qml.qrc

# Import paths for QML
QML_IMPORT_PATH += ../qml

# Additional test resources
OTHER_FILES += $$QML_FILES