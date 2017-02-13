CONFIG += release
TARGET = treemaker
QT = core gui widgets opengl
SOURCES += editor/axis.cpp editor/shared_resources.cpp editor/geometry.cpp editor/graphics.cpp editor/main.cpp editor/window.cpp editor/editor.cpp editor/camera.cpp editor/file_exporter.cpp editor/property_box.cpp editor/curve_editor.cpp editor/curve_button.cpp
HEADERS += editor/axis.h editor/shared_resources.h editor/geometry.h editor/graphics.h editor/window.h editor/editor.h editor/camera.h editor/file_exporter.h editor/property_box.h editor/curve_editor.h editor/curve_button.h
FORMS += editor/qt/window.ui
MOC_DIR = editor/qt
RCC_DIR = editor/qt
UI_DIR = editor/qt
OBJECTS_DIR = build
LIBS += -L ./lib -ltreemaker
INCLUDEPATH = ./lib/include
