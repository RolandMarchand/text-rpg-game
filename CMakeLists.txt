cmake_minimum_required(VERSION 3.10)

project(game VERSION 0.1
  DESCRIPTION "A tiny text RPG game"
  LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror=vla")

add_subdirectory(src)
