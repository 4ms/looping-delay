#pragma once
#include "drivers/adc_builtin_conf.hh"
#include "drivers/dma_config_struct.hh"
#include "drivers/pin.hh"
#include <array>
// #include "drivers/stm32xx.h"
// #include "drivers/timekeeper.hh"

namespace MetaModule
{

using mdrivlib::AdcChannelConf;
using mdrivlib::AdcChanNum;
using mdrivlib::GPIO;
using mdrivlib::PinDef;
using mdrivlib::PinNum;

// const mdrivlib::TimekeeperConfig control_read_tim_conf = {
// 	.TIMx = TIM6,
// 	.period_ns = 40000,
// 	.priority1 = 2,
// 	.priority2 = 3,
// };

struct BoardPinConf {
	static constexpr PinDef ping_but{GPIO::C, PinNum::_7};
	static constexpr PinDef hold_but{GPIO::C, PinNum::_11};
	static constexpr PinDef reverse_but{GPIO::A, PinNum::_15};

	static constexpr PinDef ping_led{GPIO::A, PinNum::_2};
	static constexpr PinDef hold_led{GPIO::I, PinNum::_0};
	static constexpr PinDef reverse_led{GPIO::D, PinNum::_2};

	static constexpr PinDef clk_led{GPIO::A, PinNum::_8};

	static constexpr PinDef ping_jack{GPIO::I, PinNum::_1};
	static constexpr PinDef hold_jack{GPIO::I, PinNum::_5};
	static constexpr PinDef reverse_jack{GPIO::B, PinNum::_8};
	static constexpr PinDef clk_out_jack{GPIO::I, PinNum::_7};
	static constexpr PinDef loop_clk_out_jack{GPIO::H, PinNum::_5};

	static constexpr PinDef time_switch_up{GPIO::C, PinNum::_8};
	static constexpr PinDef time_switch_down{GPIO::C, PinNum::_9};
};

struct BoardAdcConf : mdrivlib::DefaultAdcPeriphConf {
	static constexpr mdrivlib::AdcResolution resolution = mdrivlib::Bits12;
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

enum Pots : uint32_t { TimePot, FeedbackPot, MixPot, DelayFeedPot };
enum CVs : uint32_t { TimeCV, FeedbackCV, MixCV, DelayFeedCV };
constexpr auto AdcSampTime = mdrivlib::AdcSamplingTime::_480Cycles;
constexpr auto AdcConfs = std::to_array({
	AdcChannelConf{{GPIO::C, PinNum::_3}, AdcChanNum::_13, TimePot, AdcSampTime},
	AdcChannelConf{{GPIO::A, PinNum::_1}, AdcChanNum::_1, FeedbackPot, AdcSampTime},
	AdcChannelConf{{GPIO::A, PinNum::_0}, AdcChanNum::_0, MixPot, AdcSampTime},
	AdcChannelConf{{GPIO::C, PinNum::_4}, AdcChanNum::_14, DelayFeedPot, AdcSampTime},
	AdcChannelConf{{GPIO::C, PinNum::_0}, AdcChanNum::_10, TimeCV, AdcSampTime},
	AdcChannelConf{{GPIO::A, PinNum::_5}, AdcChanNum::_5, FeedbackCV, AdcSampTime},
	AdcChannelConf{{GPIO::A, PinNum::_4}, AdcChanNum::_4, MixCV, AdcSampTime},
	AdcChannelConf{{GPIO::C, PinNum::_1}, AdcChanNum::_11, DelayFeedCV, AdcSampTime},
});

} // namespace MetaModule
