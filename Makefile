.PHONY: build

build:
	cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cd build && make -j16
	./run.sh
