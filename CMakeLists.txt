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
  GIT_TAG 1.0.1 # Replace with the desired version tag or branch
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6-docking
)
# Fetch TinyObjLoader
FetchContent_Declare(
  tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG v2.0.0rc13  # Replace with the version you want to use
)
FetchContent_MakeAvailable(tinyobjloader)
FetchContent_MakeAvailable(imgui)

# Make GLM available
FetchContent_MakeAvailable(glm)

# Make sure the content is downloaded
FetchContent_MakeAvailable(glfw)
# Find Vulkan
find_package(Vulkan REQUIRED)

add_library(imgui
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui.h
  ${imgui_SOURCE_DIR}/imconfig.h
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_internal.h
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imstb_rectpack.h
  ${imgui_SOURCE_DIR}/imstb_textedit.h
  ${imgui_SOURCE_DIR}/imstb_truetype.h
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
		"src/*.cpp"
		"src/*.h"
)

# Add Vulkan, GLFW, and ImGui include directories
include_directories(
    ${GLFW_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    "includes/"
)

# Add executable
add_executable(${PROJECT_NAME}  ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui) 
target_link_libraries(${PROJECT_NAME} PRIVATE tinyobjloader) 
