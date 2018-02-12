TEMPLATE = app
CONFIG += qt debug object_parallel_to_source
TARGET = plant
QT = core gui opengl

SOURCES += \
plant_generator/math/curve.cpp \
plant_generator/math/intersection.cpp \
plant_generator/math/math.cpp \
plant_generator/generator.cpp \
plant_generator/mesh.cpp \
plant_generator/path.cpp \
plant_generator/plant.cpp \
plant_generator/spline.cpp \
plant_generator/stem.cpp \
plant_generator/volumetric_path.cpp \
editor/geometry/axes.cpp \
editor/geometry/geometry.cpp \
editor/geometry/path.cpp \
editor/geometry/translation_axes.cpp \
editor/geometry/rotation_axes.cpp \
editor/graphics/buffer.cpp \
editor/graphics/shared_resources.cpp \
editor/widgets/curve_button.cpp \
editor/widgets/curve_editor.cpp \
editor/widgets/editor.cpp \
editor/widgets/property_box.cpp \
editor/widgets/window.cpp \
editor/camera.cpp \
editor/closest.cpp \
editor/file_exporter.cpp \
editor/history.cpp \
editor/main.cpp \
editor/plant.cpp

HEADERS += \
plant_generator/math/curve.h \
plant_generator/math/intersection.h \
plant_generator/math/math.h \
plant_generator/generator.h \
plant_generator/mesh.h \
plant_generator/path.h \
plant_generator/patterns.h \
plant_generator/plant.h \
plant_generator/spline.h \
plant_generator/stem.h \
plant_generator/volumetric_path.h \
editor/geometry/axes.h \
editor/geometry/geometry.h \
editor/geometry/path.h \
editor/geometry/translation_axes.h \
editor/geometry/rotation_axes.h \
editor/graphics/buffer.h \
editor/graphics/shared_resources.h \
editor/widgets/curve_button.h \
editor/widgets/curve_editor.h \
editor/widgets/property_box.h \
editor/widgets/editor.h \
editor/widgets/window.h \
editor/closest.h \
editor/camera.h \
editor/file_exporter.h \
editor/history.h \
editor/plant.h

FORMS += editor/qt/window.ui
MOC_DIR = editor/qt
RCC_DIR = editor/qt
UI_DIR = editor/qt
OBJECTS_DIR = build
