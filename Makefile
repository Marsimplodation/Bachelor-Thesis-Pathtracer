.PHONY: build profile

build:
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd build && make -j16
	cd showcase && ../build/pathtracer scenes/cornel.scene

debug:
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	cd build && make -j16
	cd showcase && gdb --args ../build/pathtracer scenes/lucy.scene



profile:
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DPROFILE=ON
	cd build && make -j16
	cd showcase && ../build/pathtracer
	cd showcase && gprof ../build/pathtracer > main.gprof
	cd showcase && gprof2dot < main.gprof | dot -Tsvg -o output.svg
	firefox-developer-edition showcase/output.svg
