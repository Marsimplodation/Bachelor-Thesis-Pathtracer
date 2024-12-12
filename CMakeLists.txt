cmake_minimum_required(VERSION 3.24)
project(vkrenderer)

# Enable FetchContent module
include(FetchContent)

# Fetch GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8 # Replace with the desired version or branch
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8 # Replace with the desired version tag or branch
)

# Make GLM available
FetchContent_MakeAvailable(glm)

# Make sure the content is downloaded
FetchContent_MakeAvailable(glfw)
# Find Vulkan
find_package(Vulkan REQUIRED)
# Project name
# Set C++ standard (if it's a C++ project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
		"src/*.cpp"
		"src/*.h"
)
# Add executable
add_executable(${PROJECT_NAME}  ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
