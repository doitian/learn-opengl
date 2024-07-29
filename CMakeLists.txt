cmake_minimum_required(VERSION 3.29)
project(learn-opengl C)

find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)

add_library(glr
  glr/glr_setup.c
  glr/glr_teardown.c
  glr/glr_file.c
  glr/glr_shader.c
)

target_include_directories(glr PUBLIC glr)
target_link_libraries(glr PUBLIC glfw GLEW::GLEW)

add_executable(c3-1 src/c3-1.c)
target_link_libraries(c3-1 glr)
