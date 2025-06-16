TEMPLATE = app
TARGET = test_script_compiler
QT += testlib core qml
CONFIG += c++17 qt warn_on testcase

# Include paths
INCLUDEPATH += ../src

# Test sources
SOURCES += \
    test_script_compiler.cpp

# Include the necessary source files from main project
SOURCES += \
    ../src/Element.cpp \
    ../src/CanvasElement.cpp \
    ../src/ScriptElement.cpp \
    ../src/Node.cpp \
    ../src/Edge.cpp \
    ../src/Scripts.cpp \
    ../src/ScriptCompiler.cpp \
    ../src/ConsoleMessageRepository.cpp

# Headers
HEADERS += \
    ../src/Element.h \
    ../src/CanvasElement.h \
    ../src/ScriptElement.h \
    ../src/Node.h \
    ../src/Edge.h \
    ../src/Scripts.h \
    ../src/ScriptCompiler.h \
    ../src/ConsoleMessageRepository.h \
    ../src/Config.h \
    ../src/UniqueIdGenerator.h