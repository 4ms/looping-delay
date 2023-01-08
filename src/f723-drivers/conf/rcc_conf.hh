#pragma once

#include "drivers/stm32xx.h"

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
