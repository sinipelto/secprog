# CMakeList.txt : CMake project for Exercise1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Exercise1")

set (CMAKE_CXX_STANDARD 20)

# Add source to this project's executable.
add_executable (main "main.cpp" "main.h")

add_compile_options(-fno-stack-protector -z execstack)

# TODO: Add tests and install targets if needed.
