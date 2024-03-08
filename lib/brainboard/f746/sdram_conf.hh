#pragma once
#include "drivers/sdram_IS42S16400J-7BL.hh"
#include "drivers/sdram_config_struct.hh"

namespace Brain
{

using mdrivlib::GPIO;
using mdrivlib::PinAF;
using mdrivlib::PinNum;

constexpr inline mdrivlib::SDRAMPinNames SDRAM_pin_names = {
	.A0{GPIO::F, PinNum::_0, PinAF::AltFunc12},
	.A1{GPIO::F, PinNum::_1, PinAF::AltFunc12},
	.A2{GPIO::F, PinNum::_2, PinAF::AltFunc12},
	.A3{GPIO::F, PinNum::_3, PinAF::AltFunc12},
	.A4{GPIO::F, PinNum::_4, PinAF::AltFunc12},
	.A5{GPIO::F, PinNum::_5, PinAF::AltFunc12},
	.A6{GPIO::F, PinNum::_12, PinAF::AltFunc12},
	.A7{GPIO::F, PinNum::_13, PinAF::AltFunc12},
	.A8{GPIO::F, PinNum::_14, PinAF::AltFunc12},
	.A9{GPIO::F, PinNum::_15, PinAF::AltFunc12},
	.A10{GPIO::G, PinNum::_0, PinAF::AltFunc12},
	.A11{GPIO::G, PinNum::_1, PinAF::AltFunc12},
	.A12{GPIO::G, PinNum::_2, PinAF::AltFunc12},
	.BA0{GPIO::G, PinNum::_4, PinAF::AltFunc12},
	.BA1{GPIO::G, PinNum::_5, PinAF::AltFunc12},
	.D0{GPIO::D, PinNum::_14, PinAF::AltFunc12},
	.D1{GPIO::D, PinNum::_15, PinAF::AltFunc12},
	.D2{GPIO::D, PinNum::_0, PinAF::AltFunc12},
	.D3{GPIO::D, PinNum::_1, PinAF::AltFunc12},
	.D4{GPIO::E, PinNum::_7, PinAF::AltFunc12},
	.D5{GPIO::E, PinNum::_8, PinAF::AltFunc12},
	.D6{GPIO::E, PinNum::_9, PinAF::AltFunc12},
	.D7{GPIO::E, PinNum::_10, PinAF::AltFunc12},
	.D8{GPIO::E, PinNum::_11, PinAF::AltFunc12},
	.D9{GPIO::E, PinNum::_12, PinAF::AltFunc12},
	.D10{GPIO::E, PinNum::_13, PinAF::AltFunc12},
	.D11{GPIO::E, PinNum::_14, PinAF::AltFunc12},
	.D12{GPIO::E, PinNum::_15, PinAF::AltFunc12},
	.D13{GPIO::D, PinNum::_8, PinAF::AltFunc12},
	.D14{GPIO::D, PinNum::_9, PinAF::AltFunc12},
	.D15{GPIO::D, PinNum::_10, PinAF::AltFunc12},
	.NBL0{GPIO::E, PinNum::_0, PinAF::AltFunc12},
	.NBL1{GPIO::E, PinNum::_1, PinAF::AltFunc12},
	.SDCKE0{GPIO::B, PinNum::_5, PinAF::AltFunc12},
	.SDCLK{GPIO::G, PinNum::_8, PinAF::AltFunc12},
	.SDNCAS{GPIO::G, PinNum::_15, PinAF::AltFunc12},
	.SDNE0{GPIO::H, PinNum::_6, PinAF::AltFunc12},
	.SDNRAS{GPIO::F, PinNum::_11, PinAF::AltFunc12},
	.SDNWE{GPIO::H, PinNum::_5, PinAF::AltFunc12},
};

constexpr inline uint32_t FMC_Clock = 216; // Todo: read/calc this from RCC... query System?

constexpr inline mdrivlib::SDRAMConfig SDRAM_conf = {
	.pin_list = {SDRAM_pin_names},
	.timing = SDRAM_timing_IS42S16400J_7BL,
	.arch = SDRAM_arch_IS42S16400J_7BL,
	.size_bytes = 8 * 1024 * 1024,
};

constexpr inline mdrivlib::SDRAMBank SdramBank = mdrivlib::SDRAMBank::Bank2;
constexpr inline uint32_t SdramKernelClock = 216'000'000;

} // namespace Brain
