#pragma once
#include "conf/board_conf.hh"
#include "debug.hh"
#include "drivers/pin_change.hh"
#include "ping_methods.hh"

namespace LDKit
{

class Timer {
	uint32_t _ping_tmr = 0;
	uint32_t _ping_time = 12000;
	bool _ping_changed = true;
	Board::PingJack ping_jack;
	mdrivlib::PinChangeInt<Brain::LRClkPinChangeConf> pin_change;

	// uint32_t _clkout_tmr = 0;
	uint32_t _pingled_tmr = 0;
	uint32_t _loopled_tmr = 0;

	bool _ping_tmr_needs_reset = false;
	PingMethod &ping_method;

public:
	Timer(PingMethod &ping_method)
		: ping_method{ping_method} {
		pin_change.init([this] { inc(); });
	}

	void start() {
		pin_change.start();
	}

	void inc() {
		if (_ping_tmr_needs_reset) {
			_ping_tmr_needs_reset = false;
			_ping_tmr = 0;
		} else
			_ping_tmr++;

		_pingled_tmr++;
		_loopled_tmr++;

		ping_jack.update();
		if (ping_jack.just_went_high()) {
			Debug::Pin0::high();
			// TODO: if ping_method != last_ping_method PingMethodAlgorithm::reset();
			auto newtime = PingMethodAlgorithm::filter(_ping_time, _ping_tmr, ping_method);
			if (newtime.has_value()) {
				Debug::Pin1::high();
				_ping_time = newtime.value();
				_pingled_tmr = 0;
				_ping_changed = true;

				// TODO: see if this reduces jitter by much:
				// controls.clk_out.high();
				// controls.ping_led.high();
				// if (!modes.ping_locked)
				// 	flags.set_time_changed();
			}
			_ping_tmr = 0;
			Debug::Pin0::low();
			Debug::Pin1::low();
		}
	}

	bool take_ping_changed() {
		if (_ping_changed) {
			_ping_changed = false;
			return true;
		}
		return false;
	}

	uint32_t get_ping_time() {
		return _ping_time;
	}

	uint32_t get_ping_tmr() {
		return _ping_tmr;
	}
	void reset_ping_tmr() {
		_ping_tmr_needs_reset = true;
	}

	// uint32_t get_clkout_tmr() { return _clkout_tmr; }
	// void reset_clkout_tmr() { _clkout_tmr = 0; }

	uint32_t get_pingled_tmr() {
		return _pingled_tmr;
	}
	void reset_pingled_tmr() {
		_pingled_tmr = 0;
	}

	uint32_t get_loopled_tmr() {
		return _loopled_tmr;
	}
	void reset_loopled_tmr() {
		_loopled_tmr = 0;
	}
};

} // namespace LDKit
