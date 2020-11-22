add_compile_options(-Wno-unused-variable)

add_library(utils
  utils.cc)

target_include_directories(utils
  PRIVATE ${CMAKE_SOURCE_DIR}/src/include/utils)
