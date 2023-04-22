#pragma once
#include "brain_conf.hh"
#include "drivers/sdram.hh"
#include "drivers/stm32xx.h"
#include "sdram_conf.hh"

namespace LDKit
{

struct SystemTarget {
	static void init() {
		mdrivlib::SDRAMPeriph sdram{Brain::SDRAM_conf, Brain::SdramBank, Brain::SdramKernelClock};
		SCB_InvalidateICache();
		SCB_EnableICache();
	}

	static void restart() { NVIC_SystemReset(); }
};
} // namespace LDKit
