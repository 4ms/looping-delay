#pragma once
#include "conf/board_conf.hh"
#include "drivers/pin_change.hh"

namespace LDKit
{

class Timer {
	uint32_t _ping_tmr = 0;
	uint32_t _clkout_tmr = 0;
	uint32_t _pingled_tmr = 0;
	mdrivlib::PinChangeInt<Board::LRClkPinChangeConf> pin_change;

public:
	Timer() {
		pin_change.init([this] { inc(); });
	}

	void start() { pin_change.start(); }

	void inc() {
		_ping_tmr++;
		_clkout_tmr++;
		_pingled_tmr++;
	}

	uint32_t get_ping_tmr() { return _ping_tmr; }
	void reset_ping_tmr() { _ping_tmr = 0; }

	uint32_t get_clkout_tmr() { return _clkout_tmr; }
	void reset_clkout_tmr() { _clkout_tmr = 0; }

	uint32_t get_pingled_tmr() { return _pingled_tmr; }
	void reset_pingled_tmr() { _pingled_tmr = 0; }
};

} // namespace LDKit
