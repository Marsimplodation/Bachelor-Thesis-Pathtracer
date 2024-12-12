#include "Vulkan/VkRenderer.h"
#include <cstdlib>
#include <exception>
#include <iostream>
int main() {
    auto renderer = VkRenderer();
    try {
        renderer.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
