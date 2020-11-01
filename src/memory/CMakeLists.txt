add_library(memory
  mapping.cc)

target_include_directories(memory
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/utils
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/memory)
