add_subdirectory(debugger/)
add_subdirectory(parser/)
add_subdirectory(memory/)
add_subdirectory(utils/)

add_executable(hug main.cc)

target_link_libraries(hug
  PRIVATE debugger
  PRIVATE parser
  PRIVATE memory
  PRIVATE utils)

target_include_directories(hug
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/debugger
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/parser
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/utils
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/memory)
