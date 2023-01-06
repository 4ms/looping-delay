#pragma once
#include "debounced_switch.hh"
#include "drivers/adc_builtin_conf.hh"
#include "drivers/dma_config_struct.hh"
#include "drivers/pin.hh"
#include "drivers/switch_3pos.hh"
#include "elements.hh"
#include <array>
// #include "drivers/stm32xx.h"
// #include "drivers/timekeeper.hh"

namespace LoopingDelay
{

namespace Board
{
using mdrivlib::AdcChannelConf;
using mdrivlib::AdcChanNum;
using mdrivlib::GPIO;
using mdrivlib::PinDef;
using mdrivlib::PinNum;
using enum mdrivlib::PinPolarity;

// const mdrivlib::TimekeeperConfig control_read_tim_conf = {
// 	.TIMx = TIM6,
// 	.period_ns = 40000,
// 	.priority1 = 2,
// 	.priority2 = 3,
// };

using PingButton = mdrivlib::DebouncedSwitch<PinDef{GPIO::C, PinNum::_7}, Inverted>;
using HoldButton = mdrivlib::DebouncedSwitch<PinDef{GPIO::C, PinNum::_11}, Inverted>;
using RevButton = mdrivlib::DebouncedSwitch<PinDef{GPIO::A, PinNum::_15}, Inverted>;
using TimeSwitch = mdrivlib::Switch3Pos<PinDef{GPIO::C, PinNum::_8}, PinDef{GPIO::C, PinNum::_9}>;
using PingJack = mdrivlib::DebouncedPin<PinDef{GPIO::I, PinNum::_1}, Normal>;
using HoldJack = mdrivlib::DebouncedPin<PinDef{GPIO::I, PinNum::_5}, Normal>;
using RevJack = mdrivlib::DebouncedPin<PinDef{GPIO::B, PinNum::_8}, Normal>;

struct BoardPinConf {
	static constexpr PinDef ping_led{GPIO::A, PinNum::_2};
	static constexpr PinDef hold_led{GPIO::I, PinNum::_0};
	static constexpr PinDef reverse_led{GPIO::D, PinNum::_2};

	static constexpr PinDef clk_led{GPIO::A, PinNum::_8};

	static constexpr PinDef clk_out_jack{GPIO::I, PinNum::_7};
	static constexpr PinDef loop_clk_out_jack{GPIO::H, PinNum::_5};
};

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

static constexpr auto AdcSampTime = mdrivlib::AdcSamplingTime::_480Cycles;

constexpr std::array<AdcChannelConf, NumAdcs> AdcChans = {{
	{{GPIO::C, PinNum::_3}, AdcChanNum::_13, TimePot, AdcSampTime},
	{{GPIO::A, PinNum::_1}, AdcChanNum::_1, FeedbackPot, AdcSampTime},
	{{GPIO::A, PinNum::_0}, AdcChanNum::_0, MixPot, AdcSampTime},
	{{GPIO::C, PinNum::_4}, AdcChanNum::_14, DelayFeedPot, AdcSampTime},
	{{GPIO::C, PinNum::_0}, AdcChanNum::_10, TimeCV, AdcSampTime},
	{{GPIO::A, PinNum::_5}, AdcChanNum::_5, FeedbackCV, AdcSampTime},
	{{GPIO::A, PinNum::_4}, AdcChanNum::_4, MixCV, AdcSampTime},
	{{GPIO::C, PinNum::_1}, AdcChanNum::_11, DelayFeedCV, AdcSampTime},
}};

} // namespace Board
} // namespace LoopingDelay
