QT += core gui widgets opengl

CONFIG += c++17

SOURCES += \
    benchmark.cpp \
    CanvasBase.cpp \
    Canvas.cpp \
    GLCanvas.cpp \
    Element.cpp \
    Frame.cpp \
    Text.cpp \
    ClientRect.cpp \
    ElementList.cpp \
    Variable.cpp \
    Properties.cpp \
    FPSWidget.cpp \
    Controls.cpp

HEADERS += \
    CanvasBase.h \
    Canvas.h \
    GLCanvas.h \
    Element.h \
    Frame.h \
    Text.h \
    ClientRect.h \
    ElementList.h \
    Variable.h \
    Properties.h \
    FPSWidget.h \
    Controls.h

TARGET = canvas_benchmark
DESTDIR = ../bin