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
    ${root}/lib/mdrivlib/drivers/sdram.cc
    ${root}/lib/mdrivlib/drivers/hal_handlers.cc
    # ${root}/lib/mdrivlib/target/stm32f7xx/drivers/sai_tdm.cc
    # ${root}/lib/mdrivlib/target/stm32f7xx/drivers/interrupt_handler.cc
    # ${root}/lib/mdrivlib/target/stm32f7xx/boot/startup.s ${root}/lib/mdrivlib/target/stm32f7xx/boot/system_init.c
)

set(WAV_ENCODE_PYTHON_CMD
    # Note: flash sector size is 128kB, but we arbitrarily limit app size with
    # -g in order to
    # make the  wave file shorter
    # cmake-format: off
  python ${CMAKE_SOURCE_DIR}/src/bootloader/stm_audio_bootloader/fsk/encoder.py -s 22050 -b 16 -n 8 -z 4 -p 256 -g 40960 -k 1000 $<TARGET_FILE_DIR:${target}.elf>/${target}.bin
  # cmake-format: on
)

set_hal_sources(HAL_SOURCES stm32f7)
create_target(${target} stm32f7xx)

set(TARGET_BOOTLOADER_SOURCES)
set(TARGET_BOOTLOADER_LINK_SCRIPT ${CMAKE_SOURCE_DIR}/linker/STM32F723IETx_FLASH_BOOTLOADER.ld)
