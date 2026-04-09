cmake_minimum_required(VERSION 3.10)

project(game VERSION 0.1
  DESCRIPTION "A tiny text RPG game"
  LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# find_package(raylib 5.0 REQUIRED)

add_subdirectory(resources)
add_subdirectory(thirdparty)
add_subdirectory(src)
add_subdirectory(tests)
