exec: library
	g++ -o bluetree editor/main.cpp -I ./lib/include -L ./lib -lbluetree ; \
	chmod +x bluetree;

library:
	cd lib/; \
	gcc -c *.c -Iinclude/ -I../; \
	mkdir -p build/; \
	mv *.o build/; \
	cd build/; \
	ar rvs libbluetree.a *.o; \
	mv *.a ../; \
	cd ../../; \

clean:
	rm -rf lib/build lib/libbluetree.a bluetree;
