cmake_minimum_required(VERSION 3.10)

# Project name
project(MyMapProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add the vcpkg toolchain file to link dependencies
set(CMAKE_TOOLCHAIN_FILE "C:/Users/odinn/Desktop/vetur2025/c++/randomGeneratedMap/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Collect ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp  # if using tables
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_opengl3.cpp
)

# Add the src folder to the build (currently only main.cpp)
add_executable(MyMapProject 
    src/main.cpp
    src/MapGenerator.cpp
    src/MapMarker.cpp
    src/TextureManager.cpp
    src/Terrain.cpp
    ${IMGUI_SOURCES}
)

# Find and link the GLFW3, GLEW, and OpenGL packages via vcpkg
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)


target_link_libraries(MyMapProject PRIVATE 
    glfw 
    GLEW::GLEW  # Use GLEW::GLEW if available
    OpenGL::GL
)


# Include header files from the include directory
target_include_directories(MyMapProject PRIVATE 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image_write/stb-master
)
