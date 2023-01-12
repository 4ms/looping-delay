#pragma once
#include "drivers/sdram_48LC16M16_6A_conf.hh"
#include "drivers/sdram_AS4C16M16SA_6CN_conf.hh"
#include "drivers/sdram_AS4C16M16SA_7CN_conf.hh"
#include "drivers/sdram_W9825G6KH_75_conf.hh"
#include "drivers/sdram_config_struct.hh"

using mdrivlib::GPIO;
using mdrivlib::PinAF;
using mdrivlib::PinNum;

constexpr mdrivlib::SDRAMPinNames SDRAM_pin_names = {
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
	.SDCKE0{GPIO::H, PinNum::_7, PinAF::AltFunc12},
	.SDCLK{GPIO::G, PinNum::_8, PinAF::AltFunc12},
	.SDNCAS{GPIO::G, PinNum::_15, PinAF::AltFunc12},
	.SDNE0{GPIO::H, PinNum::_6, PinAF::AltFunc12},
	.SDNRAS{GPIO::F, PinNum::_11, PinAF::AltFunc12},
	.SDNWE{GPIO::A, PinNum::_7, PinAF::AltFunc12},
};

constexpr uint32_t FMC_Clock = 216; // Todo: read/calc this from RCC... query System?

constexpr mdrivlib::SDRAMConfig SDRAM_AS4C_T7_conf = {
	.pin_list = {SDRAM_pin_names},
	.timing = SDRAM_timing_AS4C16M16SA_7CN,
	.arch = SDRAM_arch_AS4C16M16SA_7CN,
	.connected_bank = 2,
	.fmc_kernel_clock_MHz = FMC_Clock,
};

constexpr mdrivlib::SDRAMConfig SDRAM_AS4C_T6_conf = {
	.pin_list = {SDRAM_pin_names},
	.timing = SDRAM_timing_AS4C16M16SA_6CN,
	.arch = SDRAM_arch_AS4C16M16SA_6CN,
	.connected_bank = 2,
	.fmc_kernel_clock_MHz = FMC_Clock,
};

constexpr mdrivlib::SDRAMConfig SDRAM_48LC16M16_6A_conf = {
	.pin_list = {SDRAM_pin_names},
	.timing = SDRAM_timing_48LC16M16_6A,
	.arch = SDRAM_arch_48LC16M16_6A,
	.connected_bank = 2,
	.fmc_kernel_clock_MHz = FMC_Clock,
};

constexpr mdrivlib::SDRAMConfig SDRAM_W9825_conf = {
	.pin_list = {SDRAM_pin_names},
	.timing = SDRAM_timing_W9825G6KH_75,
	.arch = SDRAM_arch_W9825G6KH_75,
	.connected_bank = 2,
	.fmc_kernel_clock_MHz = FMC_Clock,
};
