#pragma once
#include "drivers//pin_change_conf.hh"
#include "drivers/adc_builtin_conf.hh"
#include "drivers/debounced_switch.hh"
#include "drivers/dma_config_struct.hh"
#include "drivers/pin.hh"
#include "drivers/switch_3pos.hh"
#include "elements.hh"
#include <array>
// #include "drivers/stm32xx.h"
#include "drivers/timekeeper.hh"

namespace LDKit
{

namespace Board
{

// TODO: split this into separate files in conf/

using mdrivlib::AdcChannelConf;
using mdrivlib::AdcChanNum;
using mdrivlib::GPIO;
using mdrivlib::PinDef;
using mdrivlib::PinNum;
using enum mdrivlib::PinPolarity;
using enum mdrivlib::PinMode;

const mdrivlib::TimekeeperConfig param_update_task_conf = {
	.TIMx = TIM6,
	.period_ns = 1'000'000'000 / 6000, // 6kHz
	.priority1 = 2,
	.priority2 = 3,
};

using PingButton = mdrivlib::DebouncedSwitch<PinDef{GPIO::C, PinNum::_7}, Inverted>;
using HoldButton = mdrivlib::DebouncedSwitch<PinDef{GPIO::C, PinNum::_11}, Inverted>;
using RevButton = mdrivlib::DebouncedSwitch<PinDef{GPIO::A, PinNum::_15}, Inverted>;

using TimeSwitch = mdrivlib::Switch3Pos<PinDef{GPIO::C, PinNum::_8}, PinDef{GPIO::C, PinNum::_9}>;

using PingJack = mdrivlib::DebouncedPin<PinDef{GPIO::I, PinNum::_1}, Normal>;
using HoldJack = mdrivlib::DebouncedPin<PinDef{GPIO::I, PinNum::_5}, Normal>;
using RevJack = mdrivlib::DebouncedPin<PinDef{GPIO::B, PinNum::_8}, Normal>;

using PingLED = mdrivlib::FPin<GPIO::A, PinNum::_2, Output, Normal>;
using HoldLED = mdrivlib::FPin<GPIO::I, PinNum::_0, Output, Normal>;
using RevLED = mdrivlib::FPin<GPIO::D, PinNum::_2, Output, Normal>;
using ClkLED = mdrivlib::FPin<GPIO::A, PinNum::_8, Output, Normal>;

using ClkOut = mdrivlib::FPin<GPIO::I, PinNum::_7, Output, Normal>;
using LoopClkOut = mdrivlib::FPin<GPIO::H, PinNum::_5, Output, Normal>;

// ADC Conf:
struct PotAdcConf : mdrivlib::DefaultAdcPeriphConf {
	static constexpr auto resolution = mdrivlib::AdcResolution::Bits12;
	static constexpr auto adc_periph_num = mdrivlib::AdcPeriphNum::_1;
	static constexpr auto oversample = false;
	static constexpr auto clock_div = mdrivlib::AdcClockSourceDiv::APBClk_Div4;

	static constexpr bool enable_end_of_sequence_isr = true;
	static constexpr bool enable_end_of_conversion_isr = false;

	struct DmaConf : mdrivlib::DefaultAdcPeriphConf::DmaConf {
		static constexpr auto DMAx = 2;
		static constexpr auto StreamNum = 0;
		static constexpr auto RequestNum = DMA_CHANNEL_0;
		static constexpr auto dma_priority = Low;
		static constexpr IRQn_Type IRQn = DMA2_Stream0_IRQn;
		static constexpr uint32_t pri = 0;
		static constexpr uint32_t subpri = 0;
	};
};

struct CVAdcConf : mdrivlib::DefaultAdcPeriphConf {
	static constexpr auto resolution = mdrivlib::AdcResolution::Bits12;
	static constexpr auto adc_periph_num = mdrivlib::AdcPeriphNum::_2;
	static constexpr auto oversample = false;
	static constexpr auto clock_div = mdrivlib::AdcClockSourceDiv::APBClk_Div4;

	static constexpr bool enable_end_of_sequence_isr = true;
	static constexpr bool enable_end_of_conversion_isr = false;

	struct DmaConf : mdrivlib::DefaultAdcPeriphConf::DmaConf {
		static constexpr auto DMAx = 2;
		static constexpr auto StreamNum = 2;
		static constexpr auto RequestNum = DMA_CHANNEL_1;
		static constexpr auto dma_priority = Low;
		static constexpr IRQn_Type IRQn = DMA2_Stream2_IRQn;
		static constexpr uint32_t pri = 0;
		static constexpr uint32_t subpri = 0;
	};
};

constexpr inline auto AdcSampTime = mdrivlib::AdcSamplingTime::_480Cycles;

constexpr std::array<AdcChannelConf, NumPots> PotAdcChans = {{
	{{GPIO::C, PinNum::_3}, AdcChanNum::_13, TimePot, AdcSampTime},
	{{GPIO::A, PinNum::_1}, AdcChanNum::_1, FeedbackPot, AdcSampTime},
	{{GPIO::A, PinNum::_0}, AdcChanNum::_0, DelayFeedPot, AdcSampTime},
	{{GPIO::C, PinNum::_4}, AdcChanNum::_14, MixPot, AdcSampTime},
}};

constexpr std::array<AdcChannelConf, NumCVs> CVAdcChans = {{
	{{GPIO::C, PinNum::_0}, AdcChanNum::_10, TimeCV, AdcSampTime},
	{{GPIO::A, PinNum::_5}, AdcChanNum::_5, FeedbackCV, AdcSampTime},
	{{GPIO::C, PinNum::_1}, AdcChanNum::_11, DelayFeedCV, AdcSampTime},
	{{GPIO::A, PinNum::_4}, AdcChanNum::_4, MixCV, AdcSampTime},
}};

constexpr inline int16_t MinPotChange = 10;
constexpr inline int16_t MinCVChange = 10;

// memory_conf:
constexpr inline uint32_t MemoryStartAddr = 0xD0000000;
constexpr inline uint32_t MemorySizeBytes = 0x00800000;
constexpr inline uint32_t MemoryEndAddr = MemoryStartAddr + MemorySizeBytes;

using RAMSampleT = int16_t;
constexpr inline uint32_t MemorySampleSize = sizeof(RAMSampleT);
constexpr inline uint32_t MemorySamplesNum = MemorySizeBytes / MemorySampleSize;

// clock sync conf
struct LRClkPinChangeConf : mdrivlib::DefaultPinChangeConf {
	static constexpr uint32_t pin = 4;
	static constexpr GPIO port = GPIO::E;
	static constexpr bool on_rising_edge = true;
	static constexpr bool on_falling_edge = false;
	static constexpr uint32_t priority1 = 0;
	static constexpr uint32_t priority2 = 0;
};

} // namespace Board
} // namespace LDKit
