.PHONY: build profile

build:
	cd build && make clean
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd build && make -j16

profile:
	cd build && cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DPROFILE=ON
	cd build && make -j16
	build/pathtracer
	gprof build/pathtracer > main.gprof
	gprof2dot < main.gprof | dot -Tsvg -o output.svg
	firefox output.svg
