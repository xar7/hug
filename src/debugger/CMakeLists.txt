add_library(debugger
  debugger.cc
  process.cc
  breakpoint.cc)

target_include_directories(debugger
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/debugger
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/utils
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/parser
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/memory)
