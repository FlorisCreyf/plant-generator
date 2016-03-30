exec: library
	g++ editor/*.cpp -Ilib/include -L -llib/bluetree -o bluetree;
	chmod +x bluetree;

library:
	cd lib/; \
	gcc -c *.c -Iinclude/ -I../; \
	mkdir -p build/; \
	mv *.o build/; \
	cd build/; \
	ar rvs bluetree.a *.o; \
	mv *.a ../; \
	cd ../../; \

clean:
	rm -rf lib/build lib/bluetree.a bluetree;
