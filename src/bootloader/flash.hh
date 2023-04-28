#pragma once
#include "conf/flash_layout.hh"
#include "drivers/stm32xx.h"
#include <optional>
#include <span>

HAL_StatusTypeDef flash_write(std::span<const uint8_t *> data, uint32_t dst_addr);
HAL_StatusTypeDef flash_write_page(const uint8_t *data, uint32_t dst_addr, uint32_t bytes_to_write);
