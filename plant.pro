TEMPLATE = app
CONFIG += qt debug object_parallel_to_source c++11 -g
QMAKE_LFLAGS += -no-pie
TARGET = plant
QT = core gui opengl xml
LIBS += /usr/lib/x86_64-linux-gnu/libboost_serialization.a

SOURCES += \
plant_generator/thirdparty/pugixml/src/pugixml.cpp \
plant_generator/math/curve.cpp \
plant_generator/math/intersection.cpp \
plant_generator/math/mat4.cpp \
plant_generator/math/quat.cpp \
plant_generator/math/vec2.cpp \
plant_generator/math/vec3.cpp \
plant_generator/math/vec4.cpp \
plant_generator/file.cpp \
plant_generator/geometry.cpp \
plant_generator/leaf.cpp \
plant_generator/material.cpp \
plant_generator/mesh.cpp \
plant_generator/path.cpp \
plant_generator/plant.cpp \
plant_generator/pseudo_generator.cpp \
plant_generator/spline.cpp \
plant_generator/stem.cpp \
editor/commands/add_stem.cpp \
editor/commands/add_leaf.cpp \
editor/commands/command.cpp \
editor/commands/extrude_spline.cpp \
editor/commands/extrude_stem.cpp \
editor/commands/move_spline.cpp \
editor/commands/move_stem.cpp \
editor/commands/move_path.cpp \
editor/commands/remove_spline.cpp \
editor/commands/remove_stem.cpp \
editor/commands/rotate_stem.cpp \
editor/commands/save_point_selection.cpp \
editor/commands/save_selection.cpp \
editor/commands/save_stem.cpp \
editor/geometry/axes.cpp \
editor/geometry/geometry.cpp \
editor/geometry/path.cpp \
editor/geometry/translation_axes.cpp \
editor/geometry/rotation_axes.cpp \
editor/graphics/buffer.cpp \
editor/graphics/shader_params.cpp \
editor/graphics/shared_resources.cpp \
editor/widgets/curve_button.cpp \
editor/widgets/curve_editor.cpp \
editor/widgets/editor.cpp \
editor/widgets/key_editor.cpp \
editor/widgets/material_editor.cpp \
editor/widgets/material_viewer.cpp \
editor/widgets/mesh_editor.cpp \
editor/widgets/mesh_viewer.cpp \
editor/widgets/property_editor.cpp \
editor/widgets/window.cpp \
editor/camera.cpp \
editor/history.cpp \
editor/keymap.cpp \
editor/main.cpp \
editor/point_selection.cpp \
editor/selection.cpp

HEADERS += \
plant_generator/math/curve.h \
plant_generator/math/intersection.h \
plant_generator/math/mat4.h \
plant_generator/math/math.h \
plant_generator/math/quat.h \
plant_generator/math/vec2.h \
plant_generator/math/vec3.h \
plant_generator/math/vec4.h \
plant_generator/file.h \
plant_generator/geometry.h \
plant_generator/leaf.h \
plant_generator/material.h \
plant_generator/mesh.h \
plant_generator/path.h \
plant_generator/patterns.h \
plant_generator/plant.h \
plant_generator/pseudo_generator.h \
plant_generator/spline.h \
plant_generator/stem.h \
editor/commands/add_stem.h \
editor/commands/add_leaf.h \
editor/commands/command.h \
editor/commands/extrude_spline.h \
editor/commands/extrude_stem.h \
editor/commands/move_spline.h \
editor/commands/move_stem.h \
editor/commands/move_path.h \
editor/commands/remove_spline.h \
editor/commands/remove_stem.h \
editor/commands/rotate_stem.h \
editor/commands/save_point_selection.h \
editor/commands/save_selection.h \
editor/commands/save_stem.h \
editor/geometry/axes.h \
editor/geometry/geometry.h \
editor/geometry/path.h \
editor/geometry/translation_axes.h \
editor/geometry/rotation_axes.h \
editor/graphics/buffer.h \
editor/graphics/shader_params.h \
editor/graphics/shared_resources.h \
editor/widgets/curve_button.h \
editor/widgets/curve_editor.h \
editor/widgets/editor.h \
editor/widgets/item_delegate.h \
editor/widgets/key_editor.h \
editor/widgets/material_editor.h \
editor/widgets/mesh_editor.h \
editor/widgets/mesh_viewer.h \
editor/widgets/material_viewer.h \
editor/widgets/property_editor.h \
editor/widgets/window.h \
editor/camera.h \
editor/history.h \
editor/keymap.h \
editor/point_selection.h \
editor/selection.h

FORMS += editor/qt/window.ui
MOC_DIR = editor/qt
RCC_DIR = editor/qt
UI_DIR = editor/qt
OBJECTS_DIR = build
