#pragma once
#include "brain_pinout.hh"
#include "conf/sdram_conf.hh"
#include "drivers/adc_builtin_conf.hh"
#include "drivers/dma_config_struct.hh"
#include "drivers/pin.hh"
#include "drivers/pin_change_conf.hh"
#include "drivers/timekeeper.hh"
#include "drivers/uart.hh"
#include "ld.h"

namespace Brain
{
constexpr inline int16_t MinPotChange = 10;
constexpr inline int16_t MinCVChange = 10;
constexpr inline auto AdcSampTime = mdrivlib::AdcSamplingTime::_2Cycles;

// ADC Peripheral Conf:
struct CVAdcConf : mdrivlib::DefaultAdcPeriphConf {
	static constexpr auto resolution = mdrivlib::AdcResolution::Bits12;
	static constexpr auto adc_periph_num = mdrivlib::AdcPeriphNum::_1;
	static constexpr auto oversample = true;
	static constexpr auto oversampling_ratio = 256;
	static constexpr auto oversampling_right_bitshift = mdrivlib::AdcOversampleRightBitShift::Shift8Right;
	static constexpr auto clock_div = mdrivlib::PLL_Div1;

	static constexpr bool enable_end_of_sequence_isr = true;
	static constexpr bool enable_end_of_conversion_isr = false;

	struct DmaConf : mdrivlib::DefaultAdcPeriphConf::DmaConf {
		static constexpr auto DMAx = 2;
		static constexpr auto StreamNum = 7;
		static constexpr auto RequestNum = DMA_REQUEST_ADC1;
		static constexpr auto dma_priority = Low;
		static constexpr IRQn_Type IRQn = DMA2_Stream7_IRQn;
		static constexpr uint32_t pri = 0;
		static constexpr uint32_t subpri = 0;
	};
};

struct PotAdcConf : mdrivlib::DefaultAdcPeriphConf {
	static constexpr auto resolution = mdrivlib::AdcResolution::Bits12;
	static constexpr auto adc_periph_num = mdrivlib::AdcPeriphNum::_2;
	static constexpr auto oversample = true;
	static constexpr auto oversampling_ratio = 1024;
	static constexpr auto oversampling_right_bitshift = mdrivlib::AdcOversampleRightBitShift::Shift10Right;
	static constexpr auto clock_div = mdrivlib::PLL_Div1;

	static constexpr bool enable_end_of_sequence_isr = true;
	static constexpr bool enable_end_of_conversion_isr = false;

	struct DmaConf : mdrivlib::DefaultAdcPeriphConf::DmaConf {
		static constexpr auto DMAx = 2;
		static constexpr auto StreamNum = 6;
		static constexpr auto RequestNum = DMA_REQUEST_ADC2;
		static constexpr auto dma_priority = Low;
		static constexpr IRQn_Type IRQn = DMA2_Stream6_IRQn;
		static constexpr uint32_t pri = 0;
		static constexpr uint32_t subpri = 0;
	};

	static constexpr uint16_t min_value = 70;
};

// memory_conf:
const inline uint32_t MemoryStartAddr = A7_HEAP;
const inline uint32_t MemorySizeBytes = A7_HEAP_SZ;
const inline uint32_t MemoryEndAddr = MemoryStartAddr + MemorySizeBytes;

// using RAMSampleT = int16_t;
// constexpr inline uint32_t MemorySampleSize = sizeof(RAMSampleT);
// constexpr inline uint32_t MemorySamplesNum = MemorySizeBytes / MemorySampleSize;

// clock sync conf
struct LRClkPinChangeConf : mdrivlib::DefaultPinChangeConf {
	static constexpr uint32_t pin = 12;
	static constexpr mdrivlib::GPIO port = mdrivlib::GPIO::D;
	static constexpr bool on_rising_edge = true;
	static constexpr bool on_falling_edge = false;
	static constexpr uint32_t priority1 = 0;
	static constexpr uint32_t priority2 = 0;
};

constexpr inline uint32_t ParamUpdateHz = 6000;

const mdrivlib::TimekeeperConfig param_update_task_conf = {
	.TIMx = TIM6,
	.period_ns = 1'000'000'000 / ParamUpdateHz,
	.priority1 = 2,
	.priority2 = 3,
};

// SDCard
constexpr inline bool SdmmcWide = true;
constexpr inline uint32_t SdmmcPeriphNum = 1;
constexpr inline uint32_t SdmmcMaxSpeed = 32'000'000;

// Debug
constexpr inline uint32_t ConsoleUartBaseAddr = USART6_BASE;

} // namespace Brain
