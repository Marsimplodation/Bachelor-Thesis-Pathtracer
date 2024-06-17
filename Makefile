.PHONY: build profile

build:
	cd build && make clean
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd build && make -j16
