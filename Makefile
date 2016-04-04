QMAKE = qmake-qt4

all: libbluetree
	${QMAKE} -o qt.mk bluetree.pro; \
	make -f qt.mk;

libbluetree:
	cd lib/; \
	gcc -c *.c -Iinclude/ -I../; \
	mkdir -p build/; \
	mv *.o build/; \
	cd build/; \
	ar rvs libbluetree.a *.o; \
	mv *.a ../; \
	cd ../../;

clean:
	make -f qt.mk clean; \
	rm -rf lib/build lib/libbluetree.a bluetree qt.mk build;

