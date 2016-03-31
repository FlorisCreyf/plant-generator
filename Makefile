QMAKE = qmake-qt4

exec: library
	${QMAKE} -o qt.mk bluetree.pro; \
	make -f qt.mk;

library:
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
