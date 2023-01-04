cmake_minimum_required(VERSION 3.15.3)
set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(ld-kit C CXX ASM)

add_subdirectory(src/f723-drivers f723)

add_custom_target(
  tests ALL
  # COMMAND cd ${CMAKE_SOURCE_DIR}/tests && cmake -B build && cmake --build
  # build && build/tests
  COMMAND cd ${CMAKE_SOURCE_DIR}/lib/cpputil && make -f tests/Makefile
  USES_TERMINAL)
