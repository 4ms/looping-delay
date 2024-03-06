#pragma once

#define APP_FLASH_ADDR 0x08000000
#define APP_START_ADDR 0x08000000

#ifdef __cplusplus
// TODO: put APP_FLASH_ADDR etc in a .h file, which is used to preprocess linker script
// include that file here so can static_assert we use the same value
#include <cstdint>

#include "flash_sectors.hh"

// constexpr inline uint32_t BootloaderFlashAddr = get_sector_addr(0);
constexpr inline uint32_t SettingsFlashAddr = get_sector_addr(5);
constexpr inline uint32_t AppFlashAddr = get_sector_addr(0);
// constexpr inline uint32_t BootloaderReceiveAddr = get_sector_addr(6);

constexpr inline uint32_t AppStartAddr = AppFlashAddr;
// constexpr inline uint32_t BootloaderStartAddr = BootloaderFlashAddr;

static_assert(APP_FLASH_ADDR == AppFlashAddr);
static_assert(APP_START_ADDR == AppStartAddr);

#endif
