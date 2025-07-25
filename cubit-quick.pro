QT += core gui widgets qml quick quickcontrols2 quicktemplates2 network websockets
QT += webenginecore webenginequick
CONFIG += c++17
CONFIG += qmltypes
QML_IMPORT_NAME = Cubit
QML_IMPORT_MAJOR_VERSION = 1

# Allow both debug and release builds
CONFIG += debug_and_release

# Debug build configuration
CONFIG(debug, debug|release) {
    CONFIG += debug
    CONFIG += force_debug_info
    CONFIG -= separate_debug_info
    
    QMAKE_CXXFLAGS += -O0 -g -gdwarf-2
    QMAKE_CFLAGS += -O0 -g -gdwarf-2
    QMAKE_LFLAGS += -g
    
    # Disable stripping
    QMAKE_STRIP =
    CONFIG += nostrip
    
    # Generate dSYM on macOS
    macx {
        # Force DWARF with dSYM generation
        QMAKE_XCODE_DEBUG_INFORMATION_FORMAT = dwarf-with-dsym
        QMAKE_CXXFLAGS_DEBUG = -O0 -g -gdwarf-2
        QMAKE_CFLAGS_DEBUG = -O0 -g -gdwarf-2
        
        # Post-link dSYM generation
        QMAKE_POST_LINK = dsymutil \"$$OUT_PWD/cubit-quick.app/Contents/MacOS/cubit-quick\" -o \"$$OUT_PWD/cubit-quick.app.dSYM\"
    }
}

SOURCES += \
    src/main.cpp \
    src/Element.cpp \
    src/CanvasElement.cpp \
    src/DesignElement.cpp \
    src/ScriptElement.cpp \
    src/Frame.cpp \
    src/Text.cpp \
    src/platforms/web/WebTextInput.cpp \
    src/Shape.cpp \
    src/Variable.cpp \
    src/Component.cpp \
    src/ComponentInstance.cpp \
    src/ComponentVariant.cpp \
    src/FrameComponentInstance.cpp \
    src/FrameComponentVariant.cpp \
    src/TextComponentVariant.cpp \
    src/TextComponentInstance.cpp \
    src/WebTextInputComponentVariant.cpp \
    src/WebTextInputComponentInstance.cpp \
    src/ShapeComponentVariant.cpp \
    src/ShapeComponentInstance.cpp \
    src/Node.cpp \
    src/Edge.cpp \
    src/CanvasController.cpp \
    src/DesignCanvas.cpp \
    src/ElementModel.cpp \
    src/SelectionManager.cpp \
    src/ViewportCache.cpp \
    src/ConsoleMessageRepository.cpp \
    src/Application.cpp \
    src/Project.cpp \
    src/Panels.cpp \
    src/Scripts.cpp \
    src/CreationManager.cpp \
    src/HitTestService.cpp \
    src/JsonImporter.cpp \
    src/ElementTypeRegistry.cpp \
    src/ElementFactory.cpp \
    src/FrameFactory.cpp \
    src/TextFactory.cpp \
    src/WebTextInputFactory.cpp \
    src/ShapeFactory.cpp \
    src/QuadTree.cpp \
    src/Command.cpp \
    src/CommandHistory.cpp \
    src/commands/CreateDesignElementCommand.cpp \
    src/commands/CreateProjectCommand.cpp \
    src/commands/OpenProjectCommand.cpp \
    src/commands/DeleteElementsCommand.cpp \
    src/commands/DeleteProjectCommand.cpp \
    src/commands/MoveElementsCommand.cpp \
    src/commands/ResizeElementCommand.cpp \
    src/commands/SetPropertyCommand.cpp \
    src/ScriptCompiler.cpp \
    src/ScriptExecutor.cpp \
    src/ScriptGraphValidator.cpp \
    src/ScriptInvokeBuilder.cpp \
    src/ScriptFunctionRegistry.cpp \
    src/ScriptSerializer.cpp \
    src/SelectModeHandler.cpp \
    src/CreationModeHandler.cpp \
    src/LineModeHandler.cpp \
    src/ElementFilterProxy.cpp \
    src/FlexLayoutEngine.cpp \
    src/PrototypeController.cpp \
    src/DesignControlsController.cpp \
    src/AuthenticationManager.cpp \
    src/UrlSchemeHandler.cpp \
    src/StreamingAIClient.cpp \
    src/AICommandDispatcher.cpp \
    src/ProjectApiClient.cpp \
    src/FileManager.cpp \
    src/Serializer.cpp

HEADERS += \
    src/Element.h \
    src/FlexLayoutEngine.h \
    src/CanvasElement.h \
    src/DesignElement.h \
    src/ScriptElement.h \
    src/Frame.h \
    src/Text.h \
    src/platforms/web/WebTextInput.h \
    src/Shape.h \
    src/Variable.h \
    src/Component.h \
    src/ComponentInstance.h \
    src/ComponentVariant.h \
    src/FrameComponentInstance.h \
    src/FrameComponentVariant.h \
    src/TextComponentVariant.h \
    src/TextComponentInstance.h \
    src/WebTextInputComponentVariant.h \
    src/WebTextInputComponentInstance.h \
    src/ShapeComponentVariant.h \
    src/ShapeComponentInstance.h \
    src/ConnectionManager.h \
    src/PropertySyncer.h \
    src/PropertyCopier.h \
    src/Node.h \
    src/Edge.h \
    src/CanvasController.h \
    src/DesignCanvas.h \
    src/ElementModel.h \
    src/SelectionManager.h \
    src/Config.h \
    src/UniqueIdGenerator.h \
    src/HandleType.h \
    src/ViewportCache.h \
    src/ConsoleMessageRepository.h \
    src/Application.h \
    src/Project.h \
    src/Panels.h \
    src/Scripts.h \
    src/CreationManager.h \
    src/HitTestService.h \
    src/JsonImporter.h \
    src/PropertyDefinition.h \
    src/ElementTypeRegistry.h \
    src/ElementFactory.h \
    src/FrameFactory.h \
    src/TextFactory.h \
    src/WebTextInputFactory.h \
    src/ShapeFactory.h \
    src/QuadTree.h \
    src/Command.h \
    src/CommandHistory.h \
    src/commands/CreateDesignElementCommand.h \
    src/commands/CreateProjectCommand.h \
    src/commands/OpenProjectCommand.h \
    src/commands/DeleteElementsCommand.h \
    src/commands/DeleteProjectCommand.h \
    src/commands/MoveElementsCommand.h \
    src/commands/ResizeElementCommand.h \
    src/commands/SetPropertyCommand.h \
    src/ScriptCompiler.h \
    src/ScriptExecutor.h \
    src/ScriptGraphValidator.h \
    src/ScriptInvokeBuilder.h \
    src/ScriptFunctionRegistry.h \
    src/ScriptSerializer.h \
    src/IModeHandler.h \
    src/SelectModeHandler.h \
    src/CreationModeHandler.h \
    src/LineModeHandler.h \
    src/ElementFilterProxy.h \
    src/PrototypeController.h \
    src/DesignControlsController.h \
    src/AuthenticationManager.h \
    src/UrlSchemeHandler.h \
    src/StreamingAIClient.h \
    src/AICommandDispatcher.h \
    src/ProjectApiClient.h \
    src/FileManager.h \
    src/Serializer.h

RESOURCES += qml.qrc

# Add include paths for better organization
INCLUDEPATH += src
INCLUDEPATH += src/platforms/web

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# macOS specific configuration
macx {
    QMAKE_INFO_PLIST = Info.plist
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target