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
    src/Frame.cpp \
    src/Text.cpp \
    src/Html.cpp \
    src/Variable.cpp \
    src/Node.cpp \
    src/Edge.cpp \
    src/CanvasController.cpp \
    src/ElementModel.cpp \
    src/SelectionManager.cpp \
    src/ViewportCache.cpp

HEADERS += \
    src/Element.h \
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
    src/ViewportCache.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target