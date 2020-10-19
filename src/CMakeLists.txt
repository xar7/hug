add_subdirectory(debugger/)

add_executable(hug main.cc)

target_link_libraries(hug
  PRIVATE debugger)

target_include_directories(hug
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/debugger
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/utils)
