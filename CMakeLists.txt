cmake_minimum_required(VERSION 3.29)
project(learn-opengl C)

find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)

add_executable(c3-1 src/c3-1.c)
target_link_libraries(c3-1 glfw GLEW::GLEW)
