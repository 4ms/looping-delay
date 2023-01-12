#pragma once
#include "drivers/sdram_config_struct.hh"

const mdrivlib::SDRAMConfig sdram_conf{
	.pin_list = {
	},
	.timing = {
	},
	.arch = {
	},
	.connected_bank = 1,
	.fmc_kernel_clock_MHz = 100,
};
