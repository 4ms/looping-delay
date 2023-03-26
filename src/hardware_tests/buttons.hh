#pragma once
#include "conf/board_conf.hh"
#include "libhwtests/ButtonChecker.hh"

namespace LDKit::HWTests
{

struct TestButtons : IButtonChecker {
	TestButtons()
		: IButtonChecker{3} {
		reset();
		set_allowable_noise(10);
		set_min_steady_state_time(10);
	}

	bool _read_button(uint8_t channel) override {
		if (channel == 0)
			return Board::HoldButton::PinT::read();
		if (channel == 1)
			return Board::PingButton::PinT::read();
		if (channel == 2)
			return Board::RevButton::PinT::read();
		return false;
	}

	void _set_error_indicator(uint8_t channel, ErrorType err) override { Board::PingLED{}.set(err != ErrorType::None); }

	void _set_indicator(uint8_t indicator_num, bool newstate) override {
		if (indicator_num == 0)
			Board::HoldLED{}.set(newstate);
		if (indicator_num == 1)
			Board::PingLED{}.set(newstate);
		if (indicator_num == 2)
			Board::RevLED{}.set(newstate);
	}

	void _check_max_one_pin_changed() override {
		//
	}
};
} // namespace LDKit::HWTests
