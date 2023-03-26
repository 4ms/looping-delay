#pragma once
#include "hardware_tests/util.hh"
#include "libhwtests/SwitchChecker.hh"
#include "printf.h"

namespace LDKit::HWTests
{

struct TestSwitch : ISwitchChecker<1> {
	TestSwitch() = default;
	Board::TimeSwitch sw;

	SwitchPosition read_switch_state(uint32_t sw_num) override {
		auto pos = sw.read();
		return pos == mdrivlib::SwitchPos::Up	  ? SwitchPosition::Up :
			   pos == mdrivlib::SwitchPos::Down	  ? SwitchPosition::Down :
			   pos == mdrivlib::SwitchPos::Center ? SwitchPosition::Middle :
													SwitchPosition::Unknown;
	}
	void delay_ms(uint32_t ms) override { Util::delay_ms(ms); }
	void set_indicator(uint32_t sw_num, SwitchPosition pos) override {
		if (sw_num > 0)
			return;
		switch (pos) {
			case SwitchPosition::Up:
				Board::HoldLED::set(false);
				printf_("Up ");
				break;
			case SwitchPosition::Down:
				Board::PingLED::set(false);
				printf_("Down ");
				break;
			case SwitchPosition::Middle:
				Board::RevLED::set(false);
				printf_("Center ");
				break;
			case SwitchPosition::Unknown:
				Board::LoopLED::set(true);
				printf_("UNKNOWN SWITCH POSITION! ");
				break;
			case SwitchPosition::Initial:
				Board::RevLED::set(true);
				Board::PingLED::set(true);
				Board::HoldLED::set(true);
				printf_("Reading... ");
				break;
		}
	}
	void set_error_indicator(uint32_t sw_num) override { Board::LoopLED::set(true); }
};
} // namespace LDKit::HWTests
