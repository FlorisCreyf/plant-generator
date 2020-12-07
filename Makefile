QMAKE = qmake -qt5

.PHONY: clean erase debug release minimal test

debug:
	${QMAKE} CONFIG+=debug -o qt.mk plant.pro; make -f qt.mk;

release:
	${QMAKE} CONFIG+=release -o qt.mk plant.pro; make -f qt.mk;

clean:
	make -f qt.mk clean; \
	rm -rf lib/build qt.mk build;

CPPFLAGS += -Wpedantic -Wall -Wextra -g
BUILDDIR = minimal_build
LIBS = /usr/lib/x86_64-linux-gnu/libboost_program_options.a
SOURCES := $(addprefix $(BUILDDIR)/, \
plant_generator/file/collada.cpp \
plant_generator/file/wavefront.cpp \
plant_generator/file/xml_writer.cpp \
plant_generator/math/curve.cpp \
plant_generator/math/intersection.cpp \
plant_generator/math/mat4.cpp \
plant_generator/math/quat.cpp \
plant_generator/math/vec2.cpp \
plant_generator/math/vec3.cpp \
plant_generator/math/vec4.cpp \
plant_generator/animation.cpp \
plant_generator/cross_section.cpp \
plant_generator/curve.cpp \
plant_generator/parameter_tree.cpp \
plant_generator/generator.cpp \
plant_generator/geometry.cpp \
plant_generator/joint.cpp \
plant_generator/leaf.cpp \
plant_generator/material.cpp \
plant_generator/mesh.cpp \
plant_generator/path.cpp \
plant_generator/plant.cpp \
plant_generator/pseudo_generator.cpp \
plant_generator/scene.cpp \
plant_generator/spline.cpp \
plant_generator/stem.cpp \
plant_generator/stem_pool.cpp \
plant_generator/wind.cpp \
)
OBJECTS := $(SOURCES:.cpp=.o)

EXTRA_SOURCES := $(addprefix $(BUILDDIR)/, \
editor/commands/command.cpp \
editor/commands/generate.cpp \
editor/commands/remove_stem.cpp \
editor/commands/remove_spline.cpp \
editor/geometry/axes.cpp \
editor/geometry/geometry.cpp \
editor/geometry/translation_axes.cpp \
editor/selection.cpp \
editor/point_selection.cpp \
)
EXTRA_OBJECTS := $(EXTRA_SOURCES:.cpp=.o)

TEST_SOURCES := $(addprefix $(BUILDDIR)/, $(wildcard tests/*.cpp))
TEST_OBJECTS := $(TEST_SOURCES:.cpp=.o)

generator: $(OBJECTS) $(BUILDDIR)/plant_generator/main.o
	g++ $(OBJECTS) $(BUILDDIR)/plant_generator/main.o $(LIBS) $(CPPFLAGS) -o $@

test: $(OBJECTS) $(EXTRA_OBJECTS) $(TEST_OBJECTS)
	g++ $(CPPFLAGS) $(OBJECTS) $(EXTRA_OBJECTS) $(TEST_OBJECTS) $(LIBS) -o $@ -lboost_unit_test_framework

erase:
	rm -rf $(BUILDDIR)

-include $(OBJECTS:.o=.d)
-include $(EXTRA_OBJECTS:.o=.d)
-include $(TEST_OBJECTS:.o=.d)
-include $(BUILDDIR)/plant_generator/main.d

.PRECIOUS: $(BUILDDIR)/. $(BUILDDIR)%/.

$(BUILDDIR)/.:
	mkdir -p $@

$(BUILDDIR)%/.:
	mkdir -p $@

$(BUILDDIR)/plant_generator/main.o: plant_generator/main.cpp
	g++ -M -I. plant_generator/main.cpp > $(BUILDDIR)/plant_generator/main.d
	g++ $(CPPFLAGS) -c plant_generator/main.cpp -I. -o $(BUILDDIR)/plant_generator/main.o

.SECONDEXPANSION:

$(BUILDDIR)/plant_generator/%.o: plant_generator/%.cpp plant_generator/%.h | $$(@D)/.
	g++ -M -I. plant_generator/$*.cpp > $(BUILDDIR)/plant_generator/$*.d
	g++ $(CPPFLAGS) -c plant_generator/$*.cpp -I. -o $(BUILDDIR)/plant_generator/$*.o

$(BUILDDIR)/editor/%.o: editor/%.cpp editor/%.h | $$(@D)/.
	g++ -M -I. -DPG_MINIMAL editor/$*.cpp > $(BUILDDIR)/editor/$*.d
	g++ $(CPPFLAGS) -c editor/$*.cpp -I. -DPG_MINIMAL -o $(BUILDDIR)/editor/$*.o

$(BUILDDIR)/tests/%.o: tests/%.cpp | $$(@D)/.
	g++ -M -I. -DPG_MINIMAL tests/$*.cpp > $(BUILDDIR)/tests/$*.d
	g++ $(CPPFLAGS) -c tests/$*.cpp -I. -DPG_MINIMAL -o $(BUILDDIR)/tests/$*.o
