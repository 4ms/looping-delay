#pragma once
#include "drivers/stm32xx.h"

namespace LDKit
{

const RCC_OscInitTypeDef osc_conf{
	// .HSEState =
};

const RCC_ClkInitTypeDef clk_conf{
	// .ClockType = 0,
	// .SYSCLKSource = 0,
	// .AHBCLKDivider = 0,
	// .APB1CLKDivider = 0,
	// .APB2CLKDivider = 0,
};

const RCC_PeriphCLKInitTypeDef rcc_periph_conf = {};

const RCC_PeriphCLKInitTypeDef sai_rcc_clk_conf = {
	.PeriphClockSelection = RCC_PERIPHCLK_SAI1,
	.PLLSAI =
		{
			.PLLSAIN = 197,
			.PLLSAIQ = 8,
			.PLLSAIP = RCC_PLLSAIP_DIV2,
		},
	.PLLSAIDivQ = 2,
	.Sai2ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI,
};

} // namespace LDKit
