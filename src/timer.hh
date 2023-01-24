#pragma once
#include "conf/board_conf.hh"
#include "drivers/pin_change.hh"

namespace LDKit
{

class Timer {
	uint32_t _ping_tmr = 0;
	uint32_t _ping_time = 0;
	bool _ping_changed = true;
	Board::PingJack ping_jack;
	mdrivlib::PinChangeInt<Board::LRClkPinChangeConf> pin_change;

	uint32_t _clkout_tmr = 0;
	uint32_t _pingled_tmr = 0;
	uint32_t _loopled_tmr = 0;

public:
	Timer() {
		pin_change.init([this] { inc(); });
	}

	void start() { pin_change.start(); }

	void inc() {
		_ping_tmr++;
		_clkout_tmr++;
		_pingled_tmr++;
		ping_jack.update();
		if (ping_jack.just_went_high()) {
			_ping_tmr = 0;
			int32_t diff = std::abs((int32_t)_ping_time - (int32_t)_ping_tmr);
			if (diff > 10) {
				_clkout_tmr = 0;
				_pingled_tmr = 0;

				// controls.clk_out.high();
				// controls.ping_led.high();
				// if (!modes.ping_locked)
				// 	flags.set_time_changed();

				_ping_time = _ping_tmr;
				_ping_changed = true;
			}
		}
	}

	bool take_ping_changed() {
		if (_ping_changed) {
			_ping_changed = false;
			return true;
		}
		return false;
	}

	uint32_t get_ping_time() { return _ping_time; }

	uint32_t get_clkout_tmr() { return _clkout_tmr; }
	void reset_clkout_tmr() { _clkout_tmr = 0; }

	uint32_t get_pingled_tmr() { return _pingled_tmr; }
	void reset_pingled_tmr() { _pingled_tmr = 0; }

	uint32_t get_loopled_tmr() { return _loopled_tmr; }
	void reset_loopled_tmr() { _loopled_tmr = 0; }
};

} // namespace LDKit
