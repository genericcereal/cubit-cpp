#!/bin/bash
# Script to run qt-hello with proper Qt6 environment variables

export QT_QPA_PLATFORM=xcb
export QTWEBENGINEPROCESS_PATH=/usr/lib/qt6/libexec/QtWebEngineProcess

./qt-hello "$@"