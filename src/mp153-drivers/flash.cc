/*
 * flash.c -
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */

#include "conf/flash_layout.hh"
#include "drivers/stm32xx.h"
#include <optional>
#include <span>

static HAL_StatusTypeDef flash_erase(uint32_t address);

// template<std::integral T>
bool flash_read(std::span<uint8_t> data, uint32_t address) {
	// for (auto &d : data) {
	// 	d = (T)(*(/*volatile*/ T *)address);
	// 	address += sizeof(T);
	// }
	return true;
}

bool flash_read(std::span<uint32_t> data, uint32_t address) {
	// for (auto &d : data) {
	// 	d = (T)(*(/*volatile*/ T *)address);
	// 	address += sizeof(T);
	// }
	return true;
}

bool flash_erase_sector(uint32_t address) {
	// HAL_FLASH_Unlock();
	// auto status = flash_erase(address);
	// HAL_FLASH_Lock();
	// return status == HAL_OK;
	return true;
}

bool flash_write(std::span<const uint32_t> data, uint32_t address) {
	// TODO optimize for 16, 32, 64
	// for (auto d : data) {
	// 	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, d) != HAL_OK)
	// 		break;
	// 	address++;
	// }
	return true;
}

// Erases any sector whose starting address is contained in data (if any)
// Then, writes data
bool flash_erase_and_write(std::span<const uint8_t> data, uint32_t dst_addr) {
	// uint32_t bytes_to_write = data.size_bytes();
	// if (!bytes_to_write)
	// 	return HAL_OK;

	// HAL_FLASH_Unlock();

	// // Erase sector if dst_addr is a sector start
	// if (flash_erase(dst_addr) != HAL_OK) {
	// 	HAL_FLASH_Lock();
	// 	return false;
	// }

	// // Erase all sectors that start within our span of data
	// auto sec_num = get_containing_sector_num(dst_addr).value_or(-1);
	// if (sec_num == -1)
	// 	return false;

	// auto last_sec_num = get_containing_sector_num(dst_addr + bytes_to_write - 1).value_or(-1);
	// if (last_sec_num == -1)
	// 	return false;

	// while (sec_num < last_sec_num) {
	// 	if (flash_erase(get_sector_addr(sec_num++)) != HAL_OK) {
	// 		HAL_FLASH_Lock();
	// 		return false;
	// 	}
	// }

	// // Write data
	// if (!flash_write(data, dst_addr)) {
	// 	HAL_FLASH_Lock();
	// 	return false;
	// }

	// HAL_FLASH_Lock();
	return true;
}

static HAL_StatusTypeDef flash_erase(uint32_t address) {
	return HAL_OK;
	// auto sector_start = get_sector_num(address);
	// if (!sector_start)
	// 	return HAL_OK;

	// uint32_t sector = sector_start.value();
	// FLASH_EraseInitTypeDef eraseInit;
	// eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	// eraseInit.Sector = sector;
	// eraseInit.NbSectors = 1;
	// eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	// uint32_t result = 0;
	// HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInit, &result);
	// if (result != 0xFFFFFFFF)
	// 	return HAL_ERROR;
	// else {
	// 	return status;
	// }
}
