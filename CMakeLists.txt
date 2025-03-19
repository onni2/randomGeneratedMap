cmake_minimum_required(VERSION 3.10)

# Project name
project(MyMapProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add the vcpkg toolchain file to link dependencies
set(CMAKE_TOOLCHAIN_FILE "C:/Users/odinn/Desktop/vetur2025/c++/randomGeneratedMap/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Add the src folder to the build (currently only main.cpp)
add_executable(MyMapProject src/main.cpp)

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
target_include_directories(MyMapProject PRIVATE include)
