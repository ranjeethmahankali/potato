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

# Generate the texture
add_custom_command(TARGET potato
  POST_BUILD
  COMMAND python3 ${PROJECT_SOURCE_DIR}/genTexture.py ${CMAKE_BUILD_TYPE}
)

# Copy the shaders to bin dir
add_custom_command(TARGET potato
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/*.glsl ${CMAKE_CURRENT_BINARY_DIR}
)

# Unit tests
find_package(Catch2 CONFIG REQUIRED)
add_executable(mash "Tests.cpp")
target_include_directories(mash PRIVATE "./")
target_link_libraries(mash PRIVATE potatolib Catch2::Catch2 Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(mash)
