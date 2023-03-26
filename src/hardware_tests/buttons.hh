#pragma once
#include "conf/board_conf.hh"
#include "libhwtests/ButtonChecker.hh"

namespace SamplerKit::HWTests
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
			return Board::BankButton::PinT::read();
		if (channel == 1)
			return Board::PlayButton::PinT::read();
		if (channel == 2)
			return Board::RevButton::PinT::read();
		return false;
	}

	void _set_error_indicator(uint8_t channel, ErrorType err) override {
		Board::PlayLED{}.set_color(err == ErrorType::None ? Colors::off : Colors::red);
	}

	void _set_indicator(uint8_t indicator_num, bool newstate) override {
		if (indicator_num == 0)
			Board::BankLED{}.set_color(newstate ? Colors::green : Colors::off);
		if (indicator_num == 1)
			Board::PlayLED{}.set_color(newstate ? Colors::green : Colors::off);
		if (indicator_num == 2)
			Board::RevLED{}.set_color(newstate ? Colors::green : Colors::off);
	}

	void _check_max_one_pin_changed() override {
		//
	}
};
} // namespace SamplerKit::HWTests
