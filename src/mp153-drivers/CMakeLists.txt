cmake_minimum_required(VERSION 3.15)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(mp153 LANGUAGES C CXX ASM)

message("Configuring MP153")
include(${CMAKE_SOURCE_DIR}/cmake/project.cmake)

set(root ${CMAKE_SOURCE_DIR})

set(target mp153)
add_custom_target(
  stm32mp15xx_ca7_processed.ld COMMAND cp ${CMAKE_SOURCE_DIR}/linker/stm32mp15xx_ca7.ld
                                       ${CMAKE_CURRENT_BINARY_DIR}/stm32mp15xx_ca7_processed.ld
)
set(TARGET_LINK_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/stm32mp15xx_ca7_processed.ld)
# set(TARGET_LINK_SCRIPT ${CMAKE_SOURCE_DIR}/linker/stm32mp15xx_ca7.ld)

set(ARCH_DEFINES STM32MP157Cxx STM32MP1 CORE_CA7)
set(ARCH_FLAGS
    -mcpu=cortex-a7
    -mlittle-endian
    -mfpu=neon-vfpv4
    -mfloat-abi=hard
    -mthumb-interwork
    -mno-unaligned-access
    -mtune=cortex-a7
    -mvectorize-with-neon-quad
    -funsafe-math-optimizations
)

set(TARGET_INCLUDES
    ${root}/src/mp153-drivers
    ${root}/lib/brainboard/mp153
    ${root}/lib/mdrivlib/target/stm32mp1_ca7
    ${root}/lib/mdrivlib/target/stm32mp1
    ${root}/lib/CMSIS/Device/ST/STM32MP1xx/Include
    ${root}/lib/CMSIS/Core_A/Include
    ${root}/lib/STM32MP1xx_HAL_Driver/Inc
)
set(TARGET_SOURCES
    mmu_ca7.c
    aux_core_main.cc
    flash.cc
    ${root}/lib/mdrivlib/target/stm32mp1_ca7/boot/irq_ctrl.c
)

set_hal_sources(HAL_SOURCES stm32mp1)
create_target(${target} stm32mp1_ca7)
