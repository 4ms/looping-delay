cmake_minimum_required(VERSION 3.15.3)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(f723 LANGUAGES C CXX ASM)

message("Configuring f723")
include(${CMAKE_SOURCE_DIR}/cmake/project.cmake)

set(root ${CMAKE_SOURCE_DIR})

set(target 723)
set(TARGET_LINK_SCRIPT ${CMAKE_SOURCE_DIR}/linker/STM32F723IETx_FLASH.ld)
set(ARCH_DEFINES STM32F723xx STM32F7 ARM_MATH_CM7)
set(ARCH_FLAGS -mcpu=cortex-m7 -mfpu=fpv4-sp-d16 -mthumb)

set(TARGET_INCLUDES
    ${root}/src/f723-drivers/
    ${root}/lib/brainboard/f723
    ${root}/lib/mdrivlib/target/stm32f7xx
    ${root}/lib/mdrivlib/target/stm32f723
    ${root}/lib/CMSIS/Device/ST/STM32F7xx/Include
    ${root}/lib/STM32F7xx_HAL_Driver/Inc
)

set(TARGET_SOURCES
    ${root}/src/f723-drivers/flash.cc
    ${root}/lib/mdrivlib/drivers/hal_handlers.cc
    ${root}/lib/mdrivlib/drivers/sdram.cc
    ${root}/lib/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
    ${root}/lib/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c
)

set(WAV_ENCODE_PYTHON_CMD
    # cmake-format: off
    python3 ${CMAKE_SOURCE_DIR}/lib/stm_audio_bootloader/qpsk/encoder.py -s 48000 -b 12000 -c 6000 -p 256 -t stm32f4 -a 3 -g 131072 $<TARGET_FILE_DIR:${target}.elf>/${target}.bin
    # cmake-format: on
)

set_hal_sources(HAL_SOURCES stm32f7)
create_target(${target} stm32f7xx)

set(TARGET_BOOTLOADER_SOURCES
    ${root}/lib/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
    ${root}/lib/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c
    ${root}/src/bootloader/f723/flash.cc
    ${root}/lib/mdrivlib/drivers/hal_handlers.cc
)
set(TARGET_BOOTLOADER_INCLUDES
    ${root}/src/bootloader/f723
    ${root}/lib/brainboard/f723
    ${root}/lib/mdrivlib/target/stm32f7xx
    ${root}/lib/mdrivlib/target/stm32f723
    ${root}/lib/CMSIS/Device/ST/STM32F7xx/Include
    ${root}/lib/STM32F7xx_HAL_Driver/Inc
)

set(TARGET_BOOTLOADER_LINK_SCRIPT ${CMAKE_SOURCE_DIR}/linker/STM32F723IETx_FLASH_BOOTLOADER.ld)

# TODO: move set_..._hal_sources() to inside create_target
set_bootloader_hal_sources(BOOTLOADER_HAL_SOURCES stm32f7)
create_bootloader_target(${target} stm32f7xx)
