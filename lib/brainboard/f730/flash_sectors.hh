#pragma once
#include <cstdint>

constexpr uint32_t NumFlashSectors = 4;

constexpr uint32_t get_sector_addr(uint32_t sector_num) {
  constexpr uint32_t F72xxE_SECTORS[] = {
      0x08000000, // Sector 0: 32k
      0x08004000, // Sector 1: 32k
      0x08008000, // Sector 2: 32k
      0x0800C000, // Sector 3: 32k
      0x08080000, // end
  };
  return (sector_num <= NumFlashSectors) ? F72xxE_SECTORS[sector_num] : 0;
}