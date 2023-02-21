cmake_minimum_required(VERSION 3.22.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#VCPKG
if(NOT DEFINED ENV{VCPKG_PATH})
  message(FATAL_ERROR "Please clone vcpkg and set the VCPKG_PATH environment variable")
endif()

set(VCPKG_FEATURE_FLAGS versions)
set(VCPKG_TARGET_TRIPLET x64-linux)
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake)

project(potato)

find_package(glm CONFIG REQUIRED)
find_package(glew REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(potatolib
  "Position.cpp"
  "Util.cpp"
  "Move.cpp"
  "UCI.cpp"
)
target_link_libraries(potatolib PUBLIC
  glm::glm
)
target_include_directories(potatolib PRIVATE "./")

add_executable(potato
  "Command.cpp"
  "GLUtil.cpp"
  "Main.cpp"
  "View.cpp"
)
target_link_libraries(potato PRIVATE
  potatolib
  GLEW::GLEW
  glfw
  spdlog::spdlog
  spdlog::spdlog_header_only
)
target_include_directories(potato PRIVATE "./")

# Generate lookup tables
add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/Tables.h
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND python3 genTables.py > Tables.h
  DEPENDS ${PROJECT_SOURCE_DIR}/genTables.py
  COMMENT "Generating lookup tables for moves..."
)
add_custom_target(
  generate_lookup_tables
  DEPENDS ${PROJECT_SOURCE_DIR}/Tables.h
)
add_dependencies(potatolib generate_lookup_tables)

# Generate the texture
add_custom_command( # This command regenerates the texture from svg by running a python script
  OUTPUT # These files are the outputs of this command
  # When some target needs these files, cmake will run this command to generate them.
  ${CMAKE_CURRENT_BINARY_DIR}/pieces.png ${CMAKE_CURRENT_BINARY_DIR}/pieces.dat
  COMMAND python3 ${PROJECT_SOURCE_DIR}/genTexture.py ${CMAKE_BUILD_TYPE}
  DEPENDS # This command needs to be rerun whenever the svg is modified.
  ${PROJECT_SOURCE_DIR}/pieces.svg
  COMMENT "Generating texture for pieces..."
)
add_custom_target(generate_texture
  DEPENDS # Building this target will cause the following files to be generated.
  ${CMAKE_CURRENT_BINARY_DIR}/pieces.png ${CMAKE_CURRENT_BINARY_DIR}/pieces.dat
)
add_dependencies(potato generate_texture)

# Copy the shaders to bin dir
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vshader.glsl ${CMAKE_CURRENT_BINARY_DIR}/fshader.glsl
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/*.glsl ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${PROJECT_SOURCE_DIR}/vshader.glsl ${PROJECT_SOURCE_DIR}/fshader.glsl
  COMMENT "Copying shaders..."
)
add_custom_target(copy_shaders
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vshader.glsl ${CMAKE_CURRENT_BINARY_DIR}/fshader.glsl
)
add_dependencies(potato copy_shaders)

# Unit tests
find_package(Catch2 CONFIG REQUIRED)
add_executable(mash "Tests.cpp" "MoveTest.cpp")
target_include_directories(mash PRIVATE "./")
target_link_libraries(mash PRIVATE potatolib Catch2::Catch2 Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(mash)
