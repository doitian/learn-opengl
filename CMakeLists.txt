cmake_minimum_required(VERSION 3.29)
project(learn-opengl C)

find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(stb REQUIRED)
find_package(cglm REQUIRED)

add_library(glr
  glr/glr_setup.c
  glr/glr_teardown.c
  glr/glr_file.c
  glr/glr_shader.c
)

target_include_directories(glr PUBLIC glr)
target_link_libraries(glr PUBLIC glfw GLEW::GLEW)

function(add_assets target)
  set(expanded_paths "")
  foreach(relative_path ${ARGN})
    set(expanded_path "${CMAKE_CURRENT_SOURCE_DIR}/assets/${relative_path}")
    get_filename_component(relative_dir ${relative_path} DIRECTORY)

    add_custom_command(
      TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/${relative_dir}/"
      COMMAND ${CMAKE_COMMAND} -E copy "${expanded_path}" "$<TARGET_FILE_DIR:${target}>/${relative_path}"
    )
    list(APPEND expanded_paths "${expanded_path}")
  endforeach()

  # Add the compiled shaders as dependencies of the target
  add_custom_target(${target}_assets DEPENDS ${exapnded_paths})
  add_dependencies(${target} ${target}_assets)
endfunction()

add_executable(c3-1 src/c3-1.c)
target_link_libraries(c3-1 glr)

add_executable(c5-1 src/c5-1.c)
target_link_libraries(c5-1 glr)
add_assets(c5-1
  shaders/c5-1.vert
  shaders/c5-1.frag
)

add_executable(c5-2 src/c5-2.c)
target_link_libraries(c5-2 glr)
add_assets(c5-2
  shaders/c5-1.vert
  shaders/c5-1.frag
)

add_executable(c5-3 src/c5-3.c)
target_link_libraries(c5-3 glr)
add_assets(c5-3
  shaders/c5-1.vert
  shaders/c5-1.frag
  shaders/c5-3.frag
)

add_executable(c6-1 src/c6-1.c)
target_link_libraries(c6-1 glr)
add_assets(c6-1
  shaders/c6-1.vert
  shaders/c6-1.frag
)

add_executable(c6-2 src/c6-2.c)
target_link_libraries(c6-2 glr)
add_assets(c6-2
  shaders/c6-2.vert
  shaders/c6-2.frag
)

add_executable(c6-3 src/c6-3.c)
target_link_libraries(c6-3 glr)
add_assets(c6-3
  shaders/c6-3.vert
  shaders/c6-3.frag
)

add_executable(c7-1 src/c7-1.c)
target_link_libraries(c7-1 glr stb::stb)
add_assets(c7-1
  shaders/c7-1.vert
  shaders/c7-1.frag
  textures/container.jpg
  textures/awesomeface.png
)

add_executable(c7-2 src/c7-2.c)
target_link_libraries(c7-2 glr stb::stb)
add_assets(c7-2
  shaders/c7-1.vert
  shaders/c7-1.frag
  textures/container.jpg
  textures/awesomeface.png
)

add_executable(c7-3 src/c7-3.c)
target_link_libraries(c7-3 glr stb::stb)
add_assets(c7-3
  shaders/c7-1.vert
  shaders/c7-1.frag
  textures/container.jpg
  textures/awesomeface.png
)

add_executable(c7-4 src/c7-4.c)
target_link_libraries(c7-4 glr stb::stb)
add_assets(c7-4
  shaders/c7-1.vert
  shaders/c7-4.frag
  textures/container.jpg
  textures/awesomeface.png
)

add_executable(c8-1 src/c8-1.c)
target_link_libraries(c8-1 glr stb::stb cglm::cglm)
add_assets(c8-1
  shaders/c8-1.vert
  shaders/c8-1.frag
  textures/container.jpg
)
