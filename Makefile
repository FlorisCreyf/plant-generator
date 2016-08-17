QMAKE = qmake-qt5

all: libbluetree
	${QMAKE} -o qt.mk treemaker.pro; \
	make -f qt.mk;

libbluetree:
	cd lib/; \
	gcc -c *.c -Iinclude/ -I../ -g; \
	mkdir -p build/; \
	mv *.o build/; \
	cd build/; \
	ar rvs libtreemaker.a *.o; \
	mv *.a ../; \
	cd ../../;

clean:
	make -f qt.mk clean; \
	rm -rf lib/build lib/libtreemaker.a treemaker qt.mk build;
	#rm -rf lib/build treemaker lib/libtreemaker.a
