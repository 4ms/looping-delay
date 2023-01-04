cmake_minimum_required(VERSION 3.15.3)
set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(f723 C CXX ASM)

message("Configuring f723")
include(../../cmake/common.cmake)

set(target 723)
set(FAMILY_NAME stm32f7)
set(PROJECT_DRIVER_DIR ../../src/f723-drivers)
set(MDRIVLIB_TARGET_DIR ../../lib/mdrivlib/target/stm32f7xx)
set(TARGET_LINK_SCRIPT ${CMAKE_SOURCE_DIR}/linker/STM32F723IETx_FLASH.ld)
set(ARCH_FLAGS -mcpu=cortex-m7 -mfpu=fpv4-sp-d16)
set(ARCH_DEFINES STM32F723xx STM32F7 ARM_MATH_CM7)

set(TARGET_BOOTLOADER_SOURCES
    # ../../src/bootloader/gate_input_f423.cc
    # ../../src/f423-drivers/adc.cc
    # ../../stm32/HAL/stm32f4/Src/stm32f4xx_hal_adc.c
    # ../../stm32/HAL/stm32f4/Src/stm32f4xx_hal_dma.c
)
set(TARGET_BOOTLOADER_LINK_SCRIPT
    ${CMAKE_SOURCE_DIR}/linker/STM32F723IETx_FLASH_BOOTLOADER.ld)

set(WAV_ENCODE_PYTHON_CMD
    # Note: flash sector size is 128kB, but we arbitrarily limit app size with
    # -g in order to
    # make the  wave file shorter
    # cmake-format: off
  python ${CMAKE_SOURCE_DIR}/src/bootloader/stm_audio_bootloader/fsk/encoder.py -s 22050 -b 16 -n 8 -z 4 -p 256 -g 40960 -k 1000 $<TARGET_FILE_DIR:${target}.elf>/${target}.bin
  # cmake-format: on
)

set_target_sources_includes(${PROJECT_DRIVER_DIR} ${MDRIVLIB_TARGET_DIR}
                            ${FAMILY_NAME})
create_target(${target})
