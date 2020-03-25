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
main.o \
material.o \
mesh.o \
path.o \
plant.o \
spline.o \
stem.o)

plant: $(OBJECTS)
	g++ $(OBJECTS) $(LIBS) -o $@

clean:
	rm -r build

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/.:
	mkdir -p $@

$(BUILDDIR)%/.:
	mkdir -p $@

.SECONDEXPANSION:

$(BUILDDIR)/%.o: %.cpp | $$(@D)/.
	g++ -M $*.cpp > $(BUILDDIR)/$*.d
	g++ -c $(CPPFLAGS) $*.cpp -o $(BUILDDIR)/$*.o
