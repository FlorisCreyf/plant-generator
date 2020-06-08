#QMAKE = ~/Qt/5.9/gcc_64/bin/qmake
QMAKE = qmake -qt5

debug:
	${QMAKE} CONFIG+=debug -o qt.mk plant.pro; make -f qt.mk;

release:
	${QMAKE} CONFIG+=release -o qt.mk plant.pro; make -f qt.mk;

install:
	git submodule update --init --recursive;

clean:
	make -f qt.mk clean; \
	rm -rf lib/build qt.mk build;
