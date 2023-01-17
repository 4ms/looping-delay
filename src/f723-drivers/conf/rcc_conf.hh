#pragma once
#include "drivers/stm32xx.h"

namespace LDKit
{

const RCC_OscInitTypeDef osc_conf{
	.OscillatorType = RCC_OSCILLATORTYPE_HSE,
	.HSEState = RCC_HSE_ON,
	.PLL =
		{
			.PLLState = RCC_PLL_ON,
			.PLLSource = RCC_PLLSOURCE_HSE,
			.PLLM = 16,
			.PLLN = 432,
			.PLLP = RCC_PLLP_DIV2,
			.PLLQ = 9,
		},
};

const RCC_ClkInitTypeDef clk_conf{
	.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
	.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
	.AHBCLKDivider = RCC_SYSCLK_DIV1,
	.APB1CLKDivider = RCC_HCLK_DIV4,
	.APB2CLKDivider = RCC_HCLK_DIV2,
};

const RCC_PeriphCLKInitTypeDef rcc_periph_conf = {
	.PeriphClockSelection = RCC_PERIPHCLK_SAI1 | RCC_PERIPHCLK_UART4,
	.PLLSAI =
		{
			.PLLSAIN = 197,
			.PLLSAIQ = 8,
			.PLLSAIP = RCC_PLLSAIP_DIV2,
		},
	.PLLSAIDivQ = 2,
	.Sai2ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI,
	.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1,
};

} // namespace LDKit
