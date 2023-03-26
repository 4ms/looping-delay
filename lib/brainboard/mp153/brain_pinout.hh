#pragma once
#include "drivers/adc_builtin_conf.hh"
#include "drivers/pin.hh"
#include "drivers/tim_pwm.hh"

namespace Brain
{
namespace Pin
{
using PinDef = mdrivlib::PinDef;
using GPIO = mdrivlib::GPIO;
using PinNum = mdrivlib::PinNum;
using PinAF = mdrivlib::PinAF;
using AdcChanNum = mdrivlib::AdcChanNum;
using TimChannelNum = mdrivlib::TimChannelNum;

constexpr inline PinDef A0{GPIO::A, PinNum::_2};
constexpr inline PinDef A1{GPIO::A, PinNum::_6};
constexpr inline PinDef A2{GPIO::A, PinNum::_4};
constexpr inline PinDef A3{GPIO::A, PinNum::_5};
constexpr inline PinDef A4{GPIO::A, PinNum::_3};
constexpr inline PinDef A5{GPIO::A, PinNum::_7};
constexpr inline PinDef A6{GPIO::B, PinNum::_0};
constexpr inline PinDef A7{GPIO::C, PinNum::_5};
constexpr inline PinDef A8{GPIO::C, PinNum::_3};
constexpr inline PinDef A9{GPIO::C, PinNum::_0};

constexpr inline AdcChanNum A0AdcChan{AdcChanNum::_14};
constexpr inline AdcChanNum A1AdcChan{AdcChanNum::_3};
constexpr inline AdcChanNum A2AdcChan{AdcChanNum::_18};
constexpr inline AdcChanNum A3AdcChan{AdcChanNum::_19};
constexpr inline AdcChanNum A4AdcChan{AdcChanNum::_15};
constexpr inline AdcChanNum A5AdcChan{AdcChanNum::_7};
constexpr inline AdcChanNum A6AdcChan{AdcChanNum::_9};
constexpr inline AdcChanNum A7AdcChan{AdcChanNum::_8};
constexpr inline AdcChanNum A8AdcChan{AdcChanNum::_13};
constexpr inline AdcChanNum A9AdcChan{AdcChanNum::_10};

// Extra ADC inputs
constexpr inline AdcChanNum D8AdcChan{AdcChanNum::_11};
constexpr inline AdcChanNum D7AdcChan{AdcChanNum::_12};
constexpr inline AdcChanNum D11AdcChan{AdcChanNum::_5};
constexpr inline AdcChanNum D14AdcChan{AdcChanNum::_17};

constexpr inline PinDef D0{GPIO::A, PinNum::_9};
constexpr inline PinDef D1{GPIO::E, PinNum::_13};
constexpr inline PinDef D2{GPIO::B, PinNum::_7};
constexpr inline PinDef D3{GPIO::B, PinNum::_4};
constexpr inline PinDef D4{GPIO::D, PinNum::_2};
constexpr inline PinDef D5{GPIO::B, PinNum::_12}; // p3
// constexpr inline PinDef D5{GPIO::A, PinNum::_11}; //p4
constexpr inline PinDef D6{GPIO::B, PinNum::_10};
constexpr inline PinDef D7{GPIO::C, PinNum::_2};
constexpr inline PinDef D8{GPIO::C, PinNum::_1};
constexpr inline PinDef D9{GPIO::C, PinNum::_12};
constexpr inline PinDef D10{GPIO::A, PinNum::_8};
constexpr inline PinDef D11{GPIO::B, PinNum::_1};
constexpr inline PinDef D12{GPIO::E, PinNum::_4};
constexpr inline PinDef D13{GPIO::D, PinNum::_1};
constexpr inline PinDef D14{GPIO::A, PinNum::_1};
constexpr inline PinDef D15{GPIO::D, PinNum::_0};
constexpr inline PinDef D16{GPIO::C, PinNum::_8};
constexpr inline PinDef D17{GPIO::C, PinNum::_9};
constexpr inline PinDef D18{GPIO::C, PinNum::_10};
constexpr inline PinDef D19{GPIO::C, PinNum::_11};

constexpr inline PinDef D16SdmmcDat0AF = {D16.gpio, D16.pin, PinAF::AltFunc12};
constexpr inline PinDef D17SdmmcDat1AF = {D17.gpio, D17.pin, PinAF::AltFunc12};
constexpr inline PinDef D18SdmmcDat2AF = {D18.gpio, D18.pin, PinAF::AltFunc12};
constexpr inline PinDef D19SdmmcDat3AF = {D19.gpio, D19.pin, PinAF::AltFunc12};
constexpr inline PinDef D9SdmmcClkAF = {D9.gpio, D9.pin, PinAF::AltFunc12};
constexpr inline PinDef D4SdmmcCmdAF = {D4.gpio, D4.pin, PinAF::AltFunc12};

constexpr inline mdrivlib::TimChanConf D0PwmConf{
	.pin = {D0.gpio, D0.pin, PinAF::AltFunc1},
	.TIM = TIM1_BASE,
	.channum = TimChannelNum::_2,
};

constexpr inline mdrivlib::TimChanConf D1PwmConf{
	.pin = {D1.gpio, D1.pin, PinAF::AltFunc1},
	.TIM = TIM1_BASE,
	.channum = TimChannelNum::_3,
};

// p4 only: (p3 has no PWM)
// constexpr inline mdrivlib::TimChanConf D5PwmConf{
// 	.pin = {D5.gpio, D5.pin, PinAF::AltFunc1},
// 	.TIM = TIM1_BASE,
// 	.channum = TimChannelNum::_4,
// };

constexpr inline mdrivlib::TimChanConf D3PwmConf{
	.pin = {D3.gpio, D3.pin, PinAF::AltFunc2},
	.TIM = TIM3_BASE,
	.channum = TimChannelNum::_1,
};

// No D7 PWM on mp153 (PC2)

// No D8 PWM on mp1 (PC1)

constexpr inline mdrivlib::TimChanConf D10PwmConf{
	.pin = {D10.gpio, D10.pin, PinAF::AltFunc1},
	.TIM = TIM1_BASE,
	.channum = TimChannelNum::_1,
};

constexpr inline mdrivlib::TimChanConf D11PwmConf{
	.pin = {D11.gpio, D11.pin, PinAF::AltFunc2},
	.TIM = TIM3_BASE,
	.channum = TimChannelNum::_4,
};

constexpr inline mdrivlib::TimChanConf D14PwmConf{
	.pin = {D14.gpio, D14.pin, PinAF::AltFunc1},
	.TIM = TIM2_BASE,
	.channum = TimChannelNum::_2,
};

//
constexpr inline PinDef Debug0{GPIO::C, PinNum::_6};
constexpr inline PinDef Debug1{GPIO::C, PinNum::_7};
constexpr inline PinDef Debug2{GPIO::G, PinNum::_12};
constexpr inline PinDef Debug3{GPIO::E, PinNum::_11};
constexpr inline PinDef ConsoleUartTX{GPIO::C, PinNum::_6, PinAF::AltFunc7};
constexpr inline PinDef ConsoleUartRX{GPIO::C, PinNum::_7, PinAF::AltFunc7};

} // namespace Pin

} // namespace Brain
