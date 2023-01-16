#pragma once
#include "drivers/pin.hh"
#include "drivers/uart.hh"

struct Debug {
	using Pin0 = mdrivlib::FPin<mdrivlib::GPIO::H, mdrivlib::PinNum::_13>;
	using Pin1 = mdrivlib::FPin<mdrivlib::GPIO::H, mdrivlib::PinNum::_14>;
	using Pin2 = mdrivlib::FPin<mdrivlib::GPIO::H, mdrivlib::PinNum::_15>;
	using Pin3 = mdrivlib::FPin<mdrivlib::GPIO::D, mdrivlib::PinNum::_3>;
};

struct DebugConsole {
	mdrivlib::Uart<UART4_BASE> console;
	void console_init() {
		mdrivlib::Pin uart_tx{
			Debug::Pin0{}.Gpio_v, Debug::Pin0{}.PinNum_v, mdrivlib::PinMode::Alt, mdrivlib::PinAF::AltFunc8};
		mdrivlib::Pin uart_rx{
			Debug::Pin1{}.Gpio_v, Debug::Pin1{}.PinNum_v, mdrivlib::PinMode::Alt, mdrivlib::PinAF::AltFunc8};
	}
};
