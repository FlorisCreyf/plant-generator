# QMAKE = ~/Qt/5.8/gcc_64/bin/qmake
QMAKE = qmake-qt5

all: libtreemaker
	${QMAKE} -o qt.mk treemaker.pro; \
	make -f qt.mk;

libtreemaker:
	cd lib/; \
	g++ -c -g -Wall -pedantic *.cpp -Iinclude/ -I../; \
	mkdir -p build/; \
	mv *.o build/; \
	cd build/; \
	ar rvs libtreemaker.a *.o; \
	mv *.a ../; \
	cd ../../;

cleanlib:
	rm -rf lib/build lib/libtreemaker.a treemaker

clean:
	make -f qt.mk clean; \
	rm -rf lib/build lib/libtreemaker.a treemaker qt.mk build;
