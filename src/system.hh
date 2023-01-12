#pragma once
#include "conf/flash_layout.hh"
#include "conf/rcc_conf.hh"
#include "conf/sdram_conf.hh"
#include "drivers/sdram.hh"
#include "drivers/system.hh"

namespace LDKit
{

struct System {
	System() {
		mdrivlib::System::SetVectorTable(AppStartAddr);
		mdrivlib::SystemClocks::init_clocks(osc_conf, clk_conf, rcc_periph_conf);
	}

	mdrivlib::SDRAMPeriph sdram{SDRAM_AS4C_T7_conf};
};

} // namespace LDKit
