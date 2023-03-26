#pragma once
#include "conf/board_conf.hh"
#include "controls.hh"
#include "hardware_tests/util.hh"
#include "libhwtests/GateInChecker.hh"
#include "printf.h"

namespace LDKit::HWTests
{

// Patch EndOut to Play, then Rev
struct TestGateIns : IGateInChecker {
	Controls &controls;

	TestGateIns(Controls &controls)
		: IGateInChecker{3}
		, controls{controls} {}

	bool read_gate(uint8_t gate_num) override {
		if (gate_num == 0)
			return Board::HoldJack::PinT::read();
		if (gate_num == 1)
			return Board::PingJack::PinT::read();
		if (gate_num == 2)
			return Board::RevJack::PinT::read();
		return false;
	}

	void set_test_signal(bool newstate) override {
		Board::ClkOut::set(newstate);
		HAL_Delay(1);
	}

	void set_indicator(uint8_t indicate_num, bool newstate) override {
		if (indicate_num == 0)
			Board::HoldLED::set(newstate);
		if (indicate_num == 1)
			Board::PingLED::set(newstate);
		if (indicate_num == 2)
			Board::RevLED::set(newstate);
	}

	void set_error_indicator(uint8_t channel, ErrorType err) override { Board::LoopLED::set(err != ErrorType::None); }

	void signal_jack_done(uint8_t chan) override {}

	bool is_ready_to_read_jack(uint8_t chan) override { return true; }

	bool button_to_skip_step() override {
		controls.ping_button.update();
		bool skip = controls.ping_button.is_just_pressed();
		if (skip)
			printf_(" XXX SKIPPED\n");
		return skip;
	}
};
} // namespace LDKit::HWTests
