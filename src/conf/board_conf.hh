#pragma once
#include "brain_conf.hh"
#include "brain_pinout.hh"
#include "drivers/adc_builtin_conf.hh"
#include "drivers/debounced_switch.hh"
#include "drivers/switch_3pos.hh"
#include "elements.hh"
#include <array>

namespace LDKit::Board
{

using mdrivlib::AdcChannelConf;
using mdrivlib::AdcChanNum;
using mdrivlib::GPIO;
using mdrivlib::PinDef;
using mdrivlib::PinNum;
using enum mdrivlib::PinPolarity;
using enum mdrivlib::PinMode;
namespace BrainPin = Brain::Pin;

using PingButton = mdrivlib::DebouncedButton<BrainPin::D11, Inverted>;
using HoldButton = mdrivlib::DebouncedButton<BrainPin::D19, Inverted>;
using RevButton = mdrivlib::DebouncedButton<BrainPin::D10, Inverted>;

using TimeSwitch = mdrivlib::Switch3Pos<BrainPin::D16, BrainPin::D17>;

using PingJack = mdrivlib::DebouncedPin<BrainPin::D6, Normal>;
using HoldJack = mdrivlib::DebouncedPin<BrainPin::D2, Normal>;
using RevJack = mdrivlib::DebouncedPin<BrainPin::D3, Normal>;

using PingLED = mdrivlib::PinF<BrainPin::D14, Output, Normal>;
using HoldLED = mdrivlib::PinF<BrainPin::D5, Output, Normal>;
using RevLED = mdrivlib::PinF<BrainPin::D4, Output, Normal>;
using LoopLED = mdrivlib::PinF<BrainPin::D12, Output, Normal>;

using ClkOut = mdrivlib::PinF<BrainPin::D0, Output, Normal>;
using BusClkOut = mdrivlib::PinF<BrainPin::D18, Output, Normal>;

// if LoopClkBuilt and LoopClkKit are shorted, then use LoopClkKit (D15). Otherwise use LoopClkBuilt (D1)
// the pin not in use must be set to input
using LoopClkBuilt = mdrivlib::PinF<BrainPin::D1, Output, Normal>;
using LoopClkBuiltRead = mdrivlib::PinF<BrainPin::D1, Input, Normal>;

using LoopClkKit = mdrivlib::PinF<BrainPin::D15, Output, Normal>;
using LoopClkKitRead = mdrivlib::PinF<BrainPin::D15, Input, Normal>;

constexpr std::array<AdcChannelConf, NumPots> PotAdcChans = {{
	{BrainPin::A3, BrainPin::A3AdcChan, TimePot, Brain::AdcSampTime},
	{BrainPin::A5, BrainPin::A5AdcChan, FeedbackPot, Brain::AdcSampTime},
	{BrainPin::A6, BrainPin::A6AdcChan, DelayFeedPot, Brain::AdcSampTime},
	{BrainPin::A9, BrainPin::A9AdcChan, MixPot, Brain::AdcSampTime},
}};

constexpr std::array<AdcChannelConf, NumCVs> CVAdcChans = {{
	{BrainPin::A7, BrainPin::A7AdcChan, TimeCV, Brain::AdcSampTime},
	{BrainPin::A1, BrainPin::A1AdcChan, FeedbackCV, Brain::AdcSampTime},
	{BrainPin::A0, BrainPin::A0AdcChan, DelayFeedCV, Brain::AdcSampTime},
	{BrainPin::A8, BrainPin::A8AdcChan, MixCV, Brain::AdcSampTime},
}};

} // namespace LDKit::Board
