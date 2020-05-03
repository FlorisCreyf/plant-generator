CPPFLAGS += --pedantic -W -g
BUILDDIR := build
LIBS = /usr/lib/x86_64-linux-gnu/libboost_program_options.a
OBJECTS := $(addprefix $(BUILDDIR)/, \
math/curve.o \
math/intersection.o \
math/mat4.o \
math/quat.o \
math/vec2.o \
math/vec3.o \
math/vec4.o \
file.o \
generator.o \
geometry.o \
leaf.o \
material.o \
mesh.o \
path.o \
plant.o \
spline.o \
stem.o \
)
TEST_OBJECTS := $(addprefix $(BUILDDIR)/, \
tests/test_math.o \
)

plant: $(OBJECTS) $(BUILDDIR)/main.o
	g++ $(OBJECTS) $(BUILDDIR)/main.o $(LIBS) -o $@

test: $(OBJECTS) $(TEST_OBJECTS)
	g++ $(OBJECTS) $(TEST_OBJECTS) $(LIBS) -o $@

clean:
	rm -r build

-include $(OBJECTS:.o=.d)
-include $(BUILDDIR)/main.d
-include $(TEST_OBJECTS:.o=.d)

$(BUILDDIR)/.:
	mkdir -p $@

$(BUILDDIR)%/.:
	mkdir -p $@

.SECONDEXPANSION:

$(BUILDDIR)/%.o: %.cpp | $$(@D)/.
	g++ -M $*.cpp > $(BUILDDIR)/$*.d
	g++ -c $(CPPFLAGS) $*.cpp -o $(BUILDDIR)/$*.o

.PHONY: clean
