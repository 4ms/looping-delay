#pragma once
#include "conf/flash_layout.hh"
#include "conf/rcc_conf.hh"
#include "drivers/system.hh"

namespace LDKit
{

struct System {
	System() {
		mdrivlib::System::SetVectorTable(AppStartAddr);
		mdrivlib::SystemClocks::init_clocks(osc_conf, clk_conf, rcc_periph_conf);
	}
};

} // namespace LDKit
