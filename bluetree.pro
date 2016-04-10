CONFIG += release
TARGET = bluetree
QT = core gui widgets opengl
SOURCES += editor/main.cpp editor/window.cpp editor/view_editor.cpp editor/grid.cpp editor/camera.cpp
HEADERS += editor/window.h editor/view_editor.h editor/grid.h editor/camera.h
FORMS += editor/qt/window.ui
MOC_DIR = editor/qt
RCC_DIR = editor/qt
UI_DIR = editor/qt
OBJECTS_DIR = build
LIBS += -lGL -L ./lib -lbluetree
INCLUDEPATH = ./lib/include

