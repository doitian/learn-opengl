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

function(copy_shaders target)
  set(shader_source_paths "")
  foreach(shader_source_file ${ARGN})
    set(shader_source_path "${CMAKE_SOURCE_DIR}/${shader_source_file}")

    add_custom_command(
      TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/shaders/"
      COMMAND ${CMAKE_COMMAND} -E copy "${shader_source_path}" "$<TARGET_FILE_DIR:${target}>/shaders/"
    )
    list(APPEND shader_source_paths "${shader_source_path}")
  endforeach()

  # Add the compiled shaders as dependencies of the target
  add_custom_target(${target}_shader_sources DEPENDS ${shader_source_paths})
  add_dependencies(${target} ${target}_shader_sources)
endfunction()

add_executable(c3-1 src/c3-1.c)
target_link_libraries(c3-1 glr)

add_executable(c5-1 src/c5-1.c)
target_link_libraries(c5-1 glr)
copy_shaders(c5-1
  shaders/c5-1.vert
  shaders/c5-1.frag
)

add_executable(c5-2 src/c5-2.c)
target_link_libraries(c5-2 glr)
copy_shaders(c5-2
  shaders/c5-1.vert
  shaders/c5-1.frag
)

add_executable(c5-3 src/c5-3.c)
target_link_libraries(c5-3 glr)
copy_shaders(c5-3
  shaders/c5-1.vert
  shaders/c5-1.frag
  shaders/c5-3.frag
)

add_executable(c6-1 src/c6-1.c)
target_link_libraries(c6-1 glr)
copy_shaders(c6-1
  shaders/c6-1.vert
  shaders/c6-1.frag
)

add_executable(c6-2 src/c6-2.c)
target_link_libraries(c6-2 glr)
copy_shaders(c6-2
  shaders/c6-2.vert
  shaders/c6-2.frag
)
