add_subdirectory(unity)

enable_testing()

add_executable(test_graph EXCLUDE_FROM_ALL
  test_graph.c
  ${CMAKE_SOURCE_DIR}/src/graph.c
  ${CMAKE_SOURCE_DIR}/src/queue.c
)
target_link_libraries(test_graph PRIVATE unity obstack)
target_include_directories(test_graph PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME Graph COMMAND test_graph)

add_custom_target(tests
  DEPENDS test_graph
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
  COMMAND ctest -C $<CONFIG>
)
