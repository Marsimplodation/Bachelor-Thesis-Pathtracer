.PHONY: build

build:
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	cd build && make -j16
