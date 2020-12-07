add_library(parser
  dwarf.cc
  elf.cc)

target_link_libraries(parser
  PRIVATE dwarf)

target_include_directories(parser
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/parser
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/utils)
