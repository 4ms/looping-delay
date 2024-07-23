cmake_minimum_required(VERSION 3.15)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE "RelWithDebInfo")

project(
  ld-kit
  VERSION 1.0
  DESCRIPTION "Looping Delay"
  LANGUAGES C CXX ASM
)

option(BRAINBOARD1_0 "Build for Kit Brainboard v1.0" ON)
option(BRAINBOARD1_1 "Build for Kit Brainboard v1.1-rc4" ON)
option(BRAINBOARD_MP15x "Build for Brainboard MP15x" OFF)

if(BRAINBOARD1_0)
add_subdirectory(src/f723-drivers f723)
endif()

if(BRAINBOARD1_1)
add_subdirectory(src/f746-drivers f746)
endif()

if(BRAINBOARD_MP15x)
add_subdirectory(src/mp153-drivers mp153)
endif()

add_custom_target(
  tests ALL
  COMMAND cd ${PROJECT_SOURCE_DIR} && make -f tests/Makefile
  USES_TERMINAL
)

add_custom_target(
  dep_tests
  COMMAND cd ${PROJECT_SOURCE_DIR}/lib/cpputil && make --no-print-directory -f tests/Makefile
  COMMAND cd ${PROJECT_SOURCE_DIR}/lib/mdrivlib && make --no-print-directory -f tests/Makefile
  USES_TERMINAL
)
