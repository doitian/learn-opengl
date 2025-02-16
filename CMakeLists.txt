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
  glr/glr_model.c
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
  textures/awesomeface.png
)

add_executable(c8-2 src/c8-2.c)
target_link_libraries(c8-2 glr stb::stb cglm::cglm)
add_assets(c8-2
  shaders/c8-1.vert
  shaders/c8-1.frag
  textures/container.jpg
  textures/awesomeface.png
)

add_executable(c9-3 src/c9-3.c)
target_link_libraries(c9-3 glr stb::stb cglm::cglm)
add_assets(c9-3
  shaders/c9-3.vert
  shaders/c9-3.frag
  textures/container.jpg
  textures/awesomeface.png
)

add_executable(c10-1 src/c10-1.c)
target_link_libraries(c10-1 glr stb::stb cglm::cglm)
add_assets(c10-1
  shaders/c10-1.vert
  shaders/c10-1.frag
  textures/container.jpg
  textures/awesomeface.png
)

add_executable(c13-1 src/c13-1.c)
target_link_libraries(c13-1 glr cglm::cglm)
add_assets(c13-1
  shaders/c13-1.vert
  shaders/c13-1.light.frag
  shaders/c13-1.object.frag
)

add_executable(c13-2 src/c13-2.c)
target_link_libraries(c13-2 glr cglm::cglm)
add_assets(c13-2
  shaders/c13-2.vert
  shaders/c13-1.light.frag
  shaders/c13-2.object.frag
)

add_executable(c13-3 src/c13-3.c)
target_link_libraries(c13-3 glr cglm::cglm)
add_assets(c13-3
  shaders/c13-3.vert
  shaders/c13-1.light.frag
  shaders/c13-3.object.frag
)

add_executable(c14-1 src/c14-1.c)
target_link_libraries(c14-1 glr cglm::cglm)
add_assets(c14-1
  shaders/c14-1.vert
  shaders/c14-1.light.frag
  shaders/c14-1.object.frag
)

add_executable(c15-1 src/c15-1.c)
target_link_libraries(c15-1 glr stb::stb cglm::cglm)
add_assets(c15-1
  shaders/c15-1.vert
  shaders/c15-1.light.frag
  shaders/c15-1.object.frag
  textures/container2.png
  textures/container2_specular.png
)

add_executable(c17-1 src/c17-1.c)
target_link_libraries(c17-1 glr stb::stb cglm::cglm)
add_assets(c17-1
  shaders/c17-1.vert
  shaders/c17-1.light.frag
  shaders/c17-1.object.frag
  textures/container2.png
  textures/container2_specular.png
)

add_executable(c21-1 src/c21-1.c)
target_link_libraries(c21-1 glr stb::stb cglm::cglm)
add_assets(c21-1
  shaders/c21-1.vert
  shaders/c21-1.frag
  objects/backpack/backpack.obj
  objects/backpack/backpack.mtl
  objects/backpack/diffuse.jpg
  objects/backpack/specular.jpg
)

add_executable(c23-1 src/c23-1.c)
target_link_libraries(c23-1 glr stb::stb cglm::cglm)
add_assets(c23-1
  shaders/c23-1.vert
  shaders/c23-1.object.frag
  shaders/c23-1.border.frag
  textures/container2.png
  textures/container2_specular.png
)

add_executable(c35-1 src/c35-1.c)
target_link_libraries(c35-1 glr cglm::cglm)
add_assets(c35-1
  shaders/c35-1.depth.vert
  shaders/c35-1.depth.frag
  shaders/c35-1.quad.vert
  shaders/c35-1.depth-quad.frag
)
