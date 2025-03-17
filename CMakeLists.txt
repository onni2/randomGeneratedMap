# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(MyMapProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)

# Add the vcpkg toolchain file to link dependencies
set(CMAKE_TOOLCHAIN_FILE "C:/Users/odinn/Desktop/vetur2025/c++/randomGeneratedMap/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Add the src folder to the build (currently only main.cpp)
add_executable(MyMapProject src/main.cpp)

# Find and link the GLFW3 package via vcpkg
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
target_link_libraries(MyMapProject PRIVATE glfw)
target_link_libraries(MyMapProject PRIVATE OpenGL::GL)

# Include header files from the include directory
target_include_directories(MyMapProject PRIVATE include)

# If you're using other libraries from vcpkg, you can add them similarly
# For example, to use OpenGL:
# find_package(OpenGL REQUIRED)
# target_link_libraries(MyMapProject PRIVATE OpenGL::GL)
