#pragma once
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
using mdrivlib::AdcChannelConf;
using mdrivlib::AdcChanNum;
using mdrivlib::GPIO;
using mdrivlib::PinDef;
using mdrivlib::PinNum;
using enum mdrivlib::PinPolarity;
using enum mdrivlib::PinMode;

const mdrivlib::TimekeeperConfig control_read_tim_conf = {
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

using PingLED = mdrivlib::FPin<GPIO::A, PinNum::_2, Output, Inverted>;
using HoldLED = mdrivlib::FPin<GPIO::I, PinNum::_0, Output, Inverted>;
using RevLED = mdrivlib::FPin<GPIO::D, PinNum::_2, Output, Inverted>;
using ClkLED = mdrivlib::FPin<GPIO::A, PinNum::_8, Output, Inverted>;

using ClkOut = mdrivlib::FPin<GPIO::I, PinNum::_7, Output, Normal>;
using LoopClkOut = mdrivlib::FPin<GPIO::H, PinNum::_5, Output, Normal>;

struct AdcConf : mdrivlib::DefaultAdcPeriphConf {
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
	};
};

constexpr inline auto AdcSampTime = mdrivlib::AdcSamplingTime::_480Cycles;

constexpr std::array<AdcChannelConf, NumAdcs> AdcChans = {{
	{{GPIO::C, PinNum::_3}, AdcChanNum::_13, TimePot, AdcSampTime},
	{{GPIO::A, PinNum::_1}, AdcChanNum::_1, FeedbackPot, AdcSampTime},
	{{GPIO::C, PinNum::_4}, AdcChanNum::_14, DelayFeedPot, AdcSampTime},
	{{GPIO::A, PinNum::_0}, AdcChanNum::_0, MixPot, AdcSampTime},
	{{GPIO::C, PinNum::_0}, AdcChanNum::_10, TimeCV, AdcSampTime},
	{{GPIO::A, PinNum::_5}, AdcChanNum::_5, FeedbackCV, AdcSampTime},
	{{GPIO::C, PinNum::_1}, AdcChanNum::_11, DelayFeedCV, AdcSampTime},
	{{GPIO::A, PinNum::_4}, AdcChanNum::_4, MixCV, AdcSampTime},
}};

constexpr inline uint32_t ExternalMemoryStartAddr = 0xD0000000;
constexpr inline uint32_t ExternalMemorySizeBytes = 0x00800000;

using RAMSampleT = int16_t;
constexpr inline uint32_t RAMSampleSize = sizeof(RAMSampleT);

} // namespace Board
} // namespace LDKit
