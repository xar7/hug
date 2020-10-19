cmake_minimum_required (VERSION 3.9)

project (hug
  VERSION 0.1
  LANGUAGES CXX)

add_compile_options(-pedantic -Werror -Wall -Wextra -std=c++17 -g -DDEBUG)

add_subdirectory(src/)
