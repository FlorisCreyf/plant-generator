QMAKE = qmake -qt5

.PHONY: clean erase debug release minimal test

debug:
	${QMAKE} CONFIG+=debug -o qt.mk plant.pro; make -f qt.mk;

release:
	${QMAKE} CONFIG+=release -o qt.mk plant.pro; make -f qt.mk;

clean:
	make -f qt.mk clean; \
	rm -rf lib/build qt.mk build;

CXX = g++
CXXFLAGS += -Wpedantic -Wall -Wextra -g -DPG_SERIALIZE
BUILDDIR = minimal_build
LIBS = -lboost_program_options -lboost_serialization
SOURCES := $(addprefix $(BUILDDIR)/plant_generator/, \
file/collada.cpp \
file/wavefront.cpp \
file/xml_writer.cpp \
math/curve.cpp \
math/intersection.cpp \
math/mat4.cpp \
math/quat.cpp \
math/vec2.cpp \
math/vec3.cpp \
math/vec4.cpp \
animation.cpp \
cross_section.cpp \
curve.cpp \
parameter_tree.cpp \
generator.cpp \
geometry.cpp \
joint.cpp \
leaf.cpp \
material.cpp \
mesh.cpp \
path.cpp \
plant.cpp \
pattern_generator.cpp \
scene.cpp \
spline.cpp \
stem.cpp \
stem_pool.cpp \
volume.cpp \
wind.cpp \
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

gen: $(OBJECTS) $(BUILDDIR)/plant_generator/main.o
	$(CXX) $(OBJECTS) $(BUILDDIR)/plant_generator/main.o $(LIBS) $(CXXFLAGS) -o $@

test: $(OBJECTS) $(EXTRA_OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(EXTRA_OBJECTS) $(TEST_OBJECTS) $(LIBS) -o $@ -lboost_unit_test_framework

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
	$(CXX) -M -I. plant_generator/main.cpp > $(BUILDDIR)/plant_generator/main.d
	$(CXX) $(CXXFLAGS) -c plant_generator/main.cpp -I. -o $(BUILDDIR)/plant_generator/main.o

.SECONDEXPANSION:

$(BUILDDIR)/plant_generator/%.o: plant_generator/%.cpp plant_generator/%.h | $$(@D)/.
	$(CXX) -M -I. plant_generator/$*.cpp > $(BUILDDIR)/plant_generator/$*.d
	$(CXX) $(CXXFLAGS) -c plant_generator/$*.cpp -I. -o $(BUILDDIR)/plant_generator/$*.o

$(BUILDDIR)/editor/%.o: editor/%.cpp editor/%.h | $$(@D)/.
	$(CXX) -M -I. -DPG_MINIMAL editor/$*.cpp > $(BUILDDIR)/editor/$*.d
	$(CXX) $(CXXFLAGS) -c editor/$*.cpp -I. -DPG_MINIMAL -o $(BUILDDIR)/editor/$*.o

$(BUILDDIR)/tests/%.o: tests/%.cpp | $$(@D)/.
	$(CXX) -M -I. -DPG_MINIMAL tests/$*.cpp > $(BUILDDIR)/tests/$*.d
	$(CXX) $(CXXFLAGS) -c tests/$*.cpp -I. -DPG_MINIMAL -o $(BUILDDIR)/tests/$*.o
