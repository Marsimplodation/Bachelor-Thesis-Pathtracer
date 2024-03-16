.PHONY: build

build:
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release #Debug
	cd build && make -j16
