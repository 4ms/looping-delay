#pragma once
#include "drivers/sdram_IS42S16400J-7BL.hh"
#include "drivers/sdram_config_struct.hh"

namespace Brain
{
constexpr inline mdrivlib::SDRAMConfig SDRAM_conf = {
	.size_bytes = 512 * 1024 * 1024,
};
} // namespace Brain
