#pragma once
#include "conf/flash_layout.hh"
#include <optional>

// Returns the sector number that starts with the given address
// Returns nullopt if not a sector starting address
constexpr std::optional<uint32_t> get_sector_num(uint32_t address) {
	for (unsigned i = 0; i < NumFlashSectors; i++) {
		if (address == get_sector_addr(i))
			return i;
	}
	return std::nullopt;
}

// Returns the sector number that contains the given address
// Returns nullopt if not found
constexpr std::optional<uint32_t> get_containing_sector_num(uint32_t address) {
	if (address < get_sector_addr(0))
		return std::nullopt;

	for (unsigned i = 1; i <= NumFlashSectors; i++) {
		if (address < get_sector_addr(i))
			return i - 1;
	}

	return std::nullopt;
}
