#pragma once
#include "conf/flash_layout.hh"
#include "conf/rcc_conf.hh"
#include "conf/sdram_conf.hh"
#include "drivers/sdram.hh"
#include "drivers/system.hh"
#include "drivers/system_clocks.hh"

namespace LDKit
{

struct System {
	System() {
		mdrivlib::System::SetVectorTable(AppStartAddr);
		mdrivlib::SystemClocks::init_clocks(osc_conf, clk_conf, rcc_periph_conf);
		mdrivlib::SDRAMPeriph sdram{SDRAM_conf, SdramBank, SdramKernelClock};
		// SCB_CleanInvalidateDCache();
		// SCB_DisableDCache();
		SCB_InvalidateICache();
		SCB_EnableICache();
		// SCB_DisableDCache();
	}
};

} // namespace LDKit
