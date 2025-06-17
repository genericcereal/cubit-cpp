QT += core gui qml quick quickcontrols2 quicktemplates2
QT += webenginecore webenginequick
CONFIG += c++17
CONFIG += qmltypes
QML_IMPORT_NAME = Cubit
QML_IMPORT_MAJOR_VERSION = 1

# Allow both debug and release builds
CONFIG += debug_and_release

SOURCES += \
    src/main.cpp \
    src/Element.cpp \
    src/CanvasElement.cpp \
    src/DesignElement.cpp \
    src/ScriptElement.cpp \
    src/Frame.cpp \
    src/Text.cpp \
    src/Html.cpp \
    src/Variable.cpp \
    src/Node.cpp \
    src/Edge.cpp \
    src/CanvasController.cpp \
    src/ElementModel.cpp \
    src/SelectionManager.cpp \
    src/ViewportCache.cpp \
    src/ConsoleMessageRepository.cpp \
    src/Application.cpp \
    src/Canvas.cpp \
    src/Panels.cpp \
    src/Scripts.cpp \
    src/DragManager.cpp \
    src/CreationManager.cpp \
    src/HitTestService.cpp \
    src/JsonImporter.cpp \
    src/QuadTree.cpp \
    src/Command.cpp \
    src/CommandHistory.cpp \
    src/commands/CreateFrameCommand.cpp \
    src/commands/DeleteElementsCommand.cpp \
    src/commands/MoveElementsCommand.cpp \
    src/commands/ResizeElementCommand.cpp \
    src/commands/SetPropertyCommand.cpp \
    src/ScriptCompiler.cpp \
    src/ScriptExecutor.cpp \
    src/ScriptGraphValidator.cpp \
    src/ScriptInvokeBuilder.cpp \
    src/ScriptFunctionRegistry.cpp \
    src/ScriptSerializer.cpp

HEADERS += \
    src/Element.h \
    src/CanvasElement.h \
    src/DesignElement.h \
    src/ScriptElement.h \
    src/Frame.h \
    src/Text.h \
    src/Html.h \
    src/Variable.h \
    src/Node.h \
    src/Edge.h \
    src/CanvasController.h \
    src/ElementModel.h \
    src/SelectionManager.h \
    src/Config.h \
    src/UniqueIdGenerator.h \
    src/HandleType.h \
    src/ViewportCache.h \
    src/ConsoleMessageRepository.h \
    src/Application.h \
    src/Canvas.h \
    src/Panels.h \
    src/Scripts.h \
    src/DragManager.h \
    src/CreationManager.h \
    src/HitTestService.h \
    src/JsonImporter.h \
    src/QuadTree.h \
    src/Command.h \
    src/CommandHistory.h \
    src/commands/CreateFrameCommand.h \
    src/commands/DeleteElementsCommand.h \
    src/commands/MoveElementsCommand.h \
    src/commands/ResizeElementCommand.h \
    src/commands/SetPropertyCommand.h \
    src/ScriptCompiler.h \
    src/ScriptExecutor.h \
    src/ScriptGraphValidator.h \
    src/ScriptInvokeBuilder.h \
    src/ScriptFunctionRegistry.h \
    src/ScriptSerializer.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target