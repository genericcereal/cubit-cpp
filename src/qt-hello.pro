QT += core gui widgets            # same as Qt5 but driven by qmake6
QT += webenginewidgets            # if you need Qt WebEngine
CONFIG += c++17
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00

SOURCES += \
    main.cpp \
    Canvas.cpp \
    DetailPanel.cpp \
    ElementList.cpp \
    Properties.cpp \
    ActionsPanel.cpp \
    Element.cpp \
    Frame.cpp \
    Text.cpp \
    Html.cpp \
    Variable.cpp \
    ClientRect.cpp \
    Controls.cpp \
    FPSWidget.cpp \
    SelectionBox.cpp

HEADERS += \
    Config.h \
    Canvas.h \
    DetailPanel.h \
    ElementList.h \
    Properties.h \
    ActionsPanel.h \
    Element.h \
    Frame.h \
    Text.h \
    Html.h \
    Variable.h \
    ClientRect.h \
    Controls.h \
    FPSWidget.h \
    SelectionBox.h