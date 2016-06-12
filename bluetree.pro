CONFIG += release
TARGET = bluetree
QT = core gui widgets opengl
SOURCES += editor/main.cpp editor/window.cpp editor/view_editor.cpp editor/camera.cpp editor/file_exporter.cpp editor/scene.cpp editor/render_system.cpp editor/primitives.cpp
HEADERS += editor/window.h editor/view_editor.h editor/camera.h editor/file_exporter.h editor/scene.h editor/objects.h editor/render_system.h editor/primitives.h
FORMS += editor/qt/window.ui
MOC_DIR = editor/qt
RCC_DIR = editor/qt
UI_DIR = editor/qt
OBJECTS_DIR = build
LIBS += -L ./lib -lbluetree
INCLUDEPATH = ./lib/include

