#pragma once
#include "conf/rcc_conf.hh"
#include "drivers/system.hh"

namespace LDKit
{

struct System {
	System() {
		mdrivlib::System::SetVectorTable(0x08000000); // FixMe: use flash_layout.hh
		mdrivlib::SystemClocks::init_clocks(osc_conf, clk_conf, rcc_periph_conf);
	}
};

} // namespace LDKit
