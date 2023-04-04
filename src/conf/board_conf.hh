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

using PingLED = mdrivlib::FPin<BrainPin::D14.gpio, BrainPin::D14.pin, Output, Normal>;
using HoldLED = mdrivlib::FPin<BrainPin::D5.gpio, BrainPin::D5.pin, Output, Normal>;
using RevLED = mdrivlib::FPin<BrainPin::D4.gpio, BrainPin::D4.pin, Output, Normal>;
using LoopLED = mdrivlib::FPin<BrainPin::D12.gpio, BrainPin::D12.pin, Output, Normal>;

using ClkOut = mdrivlib::FPin<BrainPin::D0.gpio, BrainPin::D0.pin, Output, Normal>;
using LoopClkOut = mdrivlib::FPin<BrainPin::D15.gpio, BrainPin::D15.pin, Output, Normal>;
using BusClkOut = mdrivlib::FPin<BrainPin::D18.gpio, BrainPin::D18.pin, Output, Normal>;

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
