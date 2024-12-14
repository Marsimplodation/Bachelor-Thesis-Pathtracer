.PHONY: build

build:
	cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cd build && make -j16
	
	cd build && mkdir -p Shaders
	
	glslc src/Shaders/raygen.rgen -o build/Shaders/raygen.spv  --target-spv=spv1.4
	glslc src/Shaders/miss.rmiss -o build/Shaders/miss.spv --target-spv=spv1.4
	glslc src/Shaders/closesthit.rchit -o build/Shaders/closesthit.spv --target-spv=spv1.4

	cd build && ./vkrenderer
