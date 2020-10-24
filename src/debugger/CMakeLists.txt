add_library(debugger
  debugger.cc
  breakpoint.cc)

target_include_directories(debugger
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/debugger
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/utils)
