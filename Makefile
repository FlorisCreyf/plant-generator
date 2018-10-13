#QMAKE = ~/Qt/5.9/gcc_64/bin/qmake
QMAKE = qmake -qt5

all:
	${QMAKE} -o qt.mk plant.pro; \
	make -f qt.mk;

install:
	git submodule update --init --recursive;

clean:
	make -f qt.mk clean; \
	rm -rf lib/build plant_generator/libplantgenerator.a qt.mk build;
