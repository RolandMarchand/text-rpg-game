include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(obstack)

add_executable(${PROJECT_NAME}
  main.c
  utils.c
  graph.c
  queue.c
)
set_target_properties(${PROJECT_NAME}
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE
  obstack
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)
