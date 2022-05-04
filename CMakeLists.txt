cmake_minimum_required(VERSION 3.16)
project(plant-generator VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_XX_STANDARD 17)
set(CMAKE_XX_STANDARD_REQUIRED ON)

set(PLANT_SOURCE_FILES
	plant_generator/file/collada.cpp
	plant_generator/file/wavefront.cpp
	plant_generator/file/xml_writer.cpp
	plant_generator/math/curve.cpp
	plant_generator/math/intersection.cpp
	plant_generator/math/mat4.cpp
	plant_generator/math/quat.cpp
	plant_generator/math/vec2.cpp
	plant_generator/math/vec3.cpp
	plant_generator/math/vec4.cpp
	plant_generator/animation.cpp
	plant_generator/cross_section.cpp
	plant_generator/curve.cpp
	plant_generator/generator.cpp
	plant_generator/geometry.cpp
	plant_generator/joint.cpp
	plant_generator/leaf.cpp
	plant_generator/material.cpp
	plant_generator/mesh.cpp
	plant_generator/parameter_tree.cpp
	plant_generator/path.cpp
	plant_generator/plant.cpp
	plant_generator/pattern_generator.cpp
	plant_generator/scene.cpp
	plant_generator/spline.cpp
	plant_generator/stem.cpp
	plant_generator/stem_pool.cpp
	plant_generator/volume.cpp
	plant_generator/wind.cpp
)

set(EDITOR_SOURCE_FILES
	editor/widgets/window.ui
	editor/commands/add_stem.cpp
	editor/commands/add_leaf.cpp
	editor/commands/command.cpp
	editor/commands/extrude_spline.cpp
	editor/commands/extrude_stem.cpp
	editor/commands/generate.cpp
	editor/commands/move_spline.cpp
	editor/commands/move_stem.cpp
	editor/commands/move_path.cpp
	editor/commands/remove_spline.cpp
	editor/commands/remove_stem.cpp
	editor/commands/rotate_stem.cpp
	editor/commands/save_point_selection.cpp
	editor/commands/save_curve.cpp
	editor/commands/save_selection.cpp
	editor/commands/save_stem.cpp
	editor/geometry/axes.cpp
	editor/geometry/geometry.cpp
	editor/geometry/path.cpp
	editor/geometry/translation_axes.cpp
	editor/geometry/rotation_axes.cpp
	editor/graphics/storage_buffer.cpp
	editor/graphics/vertex_buffer.cpp
	editor/graphics/shader_params.cpp
	editor/graphics/shared_resources.cpp
	editor/widgets/curve_editor.cpp
	editor/widgets/curve_viewer.cpp
	editor/widgets/editor.cpp
	editor/widgets/form.cpp
	editor/widgets/generator_editor.cpp
	editor/widgets/key_editor.cpp
	editor/widgets/material_viewer.cpp
	editor/widgets/mesh_viewer.cpp
	editor/widgets/pattern_editor.cpp
	editor/widgets/property_editor.cpp
	editor/widgets/widgets.cpp
	editor/widgets/window.cpp
	editor/camera.cpp
	editor/history.cpp
	editor/keymap.cpp
	editor/main.cpp
	editor/point_selection.cpp
	editor/selection.cpp
	editor/selector.cpp
)

if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall --pedantic")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Build the generator
set(CMAKE_STATIC_LIBRARY_PREFIX "")
add_library(libplant ${PLANT_SOURCE_FILES})
find_library(libplant static_plant_lib)

# Build the GUI
find_package(Boost COMPONENTS serialization)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OpenGL Xml OpenGLWidgets)
add_compile_definitions(PG_SERIALIZE)
include_directories(.)
add_executable(plant ${EDITOR_SOURCE_FILES})
target_link_libraries(plant PRIVATE
	libplant
	Qt6::Widgets
	Qt::Xml
	Qt6::Gui
	Qt6::OpenGLWidgets
	Boost::serialization)
file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY keymap.xml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Add required DLLs to the build directory
if(WIN32)
	get_target_property(QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
	get_filename_component(QT_BIN "${QMAKE_EXE}" DIRECTORY)
	add_custom_command(TARGET plant POST_BUILD COMMAND ${QT_BIN}/windeployqt.exe plant.exe)
endif()

# Only build test suite for Linux
if (UNIX AND CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	set(TEST_SOURCE_FILES
		tests/test_commands.cpp
		tests/test_math.cpp
		tests/test_mesh.cpp
		tests/test_octree.cpp
		tests/test_path.cpp
		tests/test_pool.cpp
		tests/test_ptree.cpp
		tests/test_spline.cpp
		tests/test_wind.cpp
		tests/test.cpp
		# Recompile with omitted sources
		editor/commands/command.cpp
		editor/commands/generate.cpp
		editor/commands/remove_stem.cpp
		editor/commands/remove_spline.cpp
		editor/geometry/axes.cpp
		editor/geometry/geometry.cpp
		editor/geometry/translation_axes.cpp
		editor/selection.cpp
		editor/point_selection.cpp
	)

	find_package(Boost COMPONENTS unit_test_framework)
	add_executable(test ${TEST_SOURCE_FILES})
	set_target_properties(test PROPERTIES COMPILE_FLAGS "-DPG_MINIMAL")
	target_link_libraries(test PRIVATE libplant Boost::unit_test_framework)
endif()
