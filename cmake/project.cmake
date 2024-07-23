# TODO: make this src/CMakeLists.txt somehow...?

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

# ############## Common #####################

set(root ${CMAKE_SOURCE_DIR})

function(set_hal_sources sources family_name)
  string(TOUPPER ${family_name} family_name_uc)
  set(${sources}
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_adc.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_adc_ex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_cortex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_dma.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_exti.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_gpio.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_i2c.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_i2c_ex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_pwr.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_pwr_ex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_rcc.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_rcc_ex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_sai.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_tim.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_uart.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_usart.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_ll_fmc.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_ll_sdmmc.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_ll_tim.c
      PARENT_SCOPE
  )
endfunction()

function(set_bootloader_hal_sources sources family_name)
  string(TOUPPER ${family_name} family_name_uc)
  set(${sources}
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_cortex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_dma.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_gpio.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_i2c.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_i2c_ex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_pwr.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_pwr_ex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_rcc.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_rcc_ex.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_sai.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_hal_tim.c
      ${root}/lib/${family_name_uc}xx_HAL_Driver/Src/${family_name}xx_ll_tim.c
      PARENT_SCOPE
  )
endfunction()

# ############### Common commands #####################

function(create_target target driver_arch)
  message("Creating target ${target}")

  # Create <target>_ARCH: Interface library for defs/options common to all builds on this architecture
  add_library(${target}_ARCH INTERFACE)
  target_compile_definitions(${target}_ARCH INTERFACE USE_HAL_DRIVER USE_FULL_LL_DRIVER ${ARCH_DEFINES})

  target_compile_options(
    ${target}_ARCH
    INTERFACE $<$<CONFIG:Debug>:-O0
              -g3>
              $<$<CONFIG:Release>:-Ofast>
              $<$<CONFIG:RelWithDebInfo>:-Ofast
              -g3>
              -fdata-sections
              -ffunction-sections
              -fno-common
              -ffreestanding
              -fno-unwind-tables
              -mfloat-abi=hard
              -nostartfiles
              -nostdlib
              -Wdouble-promotion
              -Werror=return-type
              -Wall
              $<$<COMPILE_LANGUAGE:CXX>:
              -std=c++23
              -ffold-simple-inlines
              -fno-rtti
              -fno-threadsafe-statics
              -fno-exceptions
              -Wno-register
              -Wno-volatile
              >
              ${ARCH_FLAGS}
  )

  target_link_options(
    ${target}_ARCH
    INTERFACE
    -Wl,--gc-sections
    -nostdlib
    -mfloat-abi=hard
    ${ARCH_FLAGS}
  )

  # Create main app elf file target, and link to the ARCH interface
  add_executable(
    ${target}.elf
    ${root}/lib/mdrivlib/drivers/pin.cc
    ${root}/lib/mdrivlib/drivers/tim.cc
    ${root}/lib/mdrivlib/drivers/timekeeper.cc
    ${root}/lib/mdrivlib/drivers/i2c.cc
    ${root}/lib/mdrivlib/drivers/codec_PCM3060.cc
    ${root}/lib/mdrivlib/target/${driver_arch}/drivers/sai_tdm.cc
    ${root}/lib/mdrivlib/target/${driver_arch}/drivers/interrupt_handler.cc
    ${root}/lib/mdrivlib/target/${driver_arch}/boot/startup.s
    ${root}/lib/mdrivlib/target/${driver_arch}/boot/system_init.c
    ${root}/lib/libhwtests/src/AdcChecker.cc
    ${root}/lib/libhwtests/src/AdcRangeChecker.cc
    ${root}/lib/libhwtests/src/ButtonChecker.cc
    ${root}/lib/libhwtests/src/CodecCallbacks.cc
    ${root}/lib/libhwtests/src/GateInChecker.cc
    ${root}/lib/libhwtests/src/GateOutChecker.cc
    ${root}/lib/libhwtests/src/GateOutput.cc
    ${root}/lib/libhwtests/src/LEDTester.cc
    ${root}/src/libc_stub.c
    ${root}/src/libcpp_stub.cc
    ${root}/src/main.cc
    ${root}/src/console.cc
    ${root}/src/hardware_tests/hardware_tests.cc
    ${root}/src/calibration_storage.cc
    # Printf:
    ${root}/lib/printf/printf.c
    ${TARGET_SOURCES}
    ${HAL_SOURCES}
  )

  target_include_directories(
    ${target}.elf
    PRIVATE ${root}/src
            ${root}/src/hardware_tests
            ${root}/lib/brainboard
            ${root}/lib/mdrivlib
            ${root}/lib/CMSIS/Include
            ${root}/lib/cpputil
            ${TARGET_INCLUDES}
  )

  target_link_libraries(${target}.elf PRIVATE ${target}_ARCH)
  target_link_script(${target} ${TARGET_LINK_SCRIPT})
  add_bin_hex_command(${target})

  # Create libhwtests target, and link to the ARCH interface, and link main app to it
  add_subdirectory(../../lib/libhwtests ${CMAKE_CURRENT_BINARY_DIR}/libhwtests)
  target_link_libraries(libhwtests${target} PRIVATE ${target}_ARCH)
  target_link_libraries(${target}.elf PRIVATE libhwtests${target})

  # Helper for letting lsp servers know what target we're using
  add_custom_target(${target}-compdb COMMAND ln -snf ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/.)

endfunction()

function(create_bootloader_target target driver_arch)
  message("Creating bootloader for target ${target}")

  # Create bootloader elf file target
  add_executable(
    ${target}-bootloader.elf
    ${root}/src/bootloader/main.cc
    ${root}/src/bootloader/animation.cc
    ${root}/lib/stm_audio_bootloader/qpsk/packet_decoder.cc
    ${root}/lib/stm_audio_bootloader/qpsk/demodulator.cc
    ${root}/src/libc_stub.c
    ${root}/src/libcpp_stub.cc
    ${root}/lib/mdrivlib/drivers/pin.cc
    ${root}/lib/mdrivlib/drivers/timekeeper.cc
    ${root}/lib/mdrivlib/drivers/tim.cc
    ${root}/lib/mdrivlib/drivers/i2c.cc
    ${root}/lib/mdrivlib/drivers/codec_PCM3060.cc
    ${root}/lib/mdrivlib/target/${driver_arch}/drivers/sai_tdm.cc
    ${root}/lib/mdrivlib/target/${driver_arch}/drivers/interrupt_handler.cc
    ${root}/lib/mdrivlib/target/${driver_arch}/boot/startup.s
    ${root}/lib/mdrivlib/target/${driver_arch}/boot/system_init.c
    ${TARGET_BOOTLOADER_SOURCES}
    ${BOOTLOADER_HAL_SOURCES}
  )

  target_include_directories(
    ${target}-bootloader.elf
    PRIVATE ${root}/lib/CMSIS/Include
            ${root}/src/bootloader
            ${root}/src/bootloader/stmlib
            ${root}/src
            ${root}/lib
            ${root}/lib/brainboard
            ${root}/lib/mdrivlib
            ${root}/lib/cpputil
            ${TARGET_BOOTLOADER_INCLUDES}
  )

  target_link_libraries(${target}-bootloader.elf PRIVATE ${target}_ARCH)
  target_link_script(${target}-bootloader ${TARGET_BOOTLOADER_LINK_SCRIPT})
  add_bin_hex_command(${target}-bootloader)

  # Target: XXX-wav: Create .wav file for distributing firmware upgrades
  add_custom_target(
    ${target}.wav
    DEPENDS ${target}.elf
    COMMAND export PYTHONPATH="${CMAKE_SOURCE_DIR}/lib" && ${WAV_ENCODE_PYTHON_CMD}
  )

  set(TARGET_BASE $<TARGET_FILE_DIR:${target}.elf>/${target})

  # Target: XXX-combo: Create a hex file containing bootloader and app, that can be loaded via USB DFU
  add_custom_target(
    ${target}-combo
    DEPENDS ${TARGET_BASE}.hex ${TARGET_BASE}-bootloader.elf
    COMMAND cat ${TARGET_BASE}-bootloader.hex ${TARGET_BASE}.hex | awk -f ${CMAKE_SOURCE_DIR}/scripts/merge_hex.awk >
            ${TARGET_BASE}-combo.hex
    COMMAND arm-none-eabi-objcopy -O binary -I ihex ${TARGET_BASE}-combo.hex ${TARGET_BASE}-combo.bin
  )
  set_target_properties(${target}-combo PROPERTIES ADDITIONAL_CLEAN_FILES "${TARGET_BASE}-combo.hex")

  # Target: XXX-flash: Flashes bootloader and app to chip. Requires JFlashExe to be executable and in your $PATH
  add_custom_target(
    ${target}-flash
    DEPENDS ${target}-combo
    COMMAND echo "JFlashExe -openprj${CMAKE_SOURCE_DIR}/${target}.jflash -open${TARGET_BASE}-combo.hex -auto -exit"
    COMMAND JFlashExe -openprj${CMAKE_SOURCE_DIR}/${target}.jflash -open${TARGET_BASE}-combo.hex -auto -exit
    USES_TERMINAL
  )

endfunction()
