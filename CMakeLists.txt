cmake_minimum_required(VERSION 3.15)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo")

set(CMAKE_BUILD_TYPE
    "Debug"
    CACHE STRING "Choose the type of build."
)

project(
  ld-kit
  VERSION 0.1
  DESCRIPTION "Looping Delay Kit"
  LANGUAGES C CXX ASM
)

add_subdirectory(src/f723-drivers f723)
add_subdirectory(src/mp153-drivers mp153)

add_custom_target(
  tests ALL
  # COMMAND cd ${CMAKE_SOURCE_DIR}/tests && cmake -B build && cmake --build
  # build && build/tests
  COMMAND cd ${CMAKE_SOURCE_DIR}/lib/cpputil && make --no-print-directory -f tests/Makefile
  COMMAND cd ${CMAKE_SOURCE_DIR}/lib/cpputil && make -f tests/Makefile
  USES_TERMINAL
)
