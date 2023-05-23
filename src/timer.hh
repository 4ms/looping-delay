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

	Board::LoopClkOut loop_out;
	Board::LoopClkPassiveIn loop_passive;
	Board::ClkOut clk_out;
	Board::BusClkOut bus_clk_out;
	Board::LoopLED loop_led;
	bool _ping_led_high = false;

	// uint32_t _clkout_tmr = 0;
	uint32_t _pingled_tmr = 0;
	uint32_t _loopled_tmr = 0;

	uint32_t _loopled_time = 0;

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
			clk_out.high();
			bus_clk_out.high();
		} else
			_ping_tmr++;

		_pingled_tmr++;
		_loopled_tmr++;

		ping_jack.update();
		if (ping_jack.just_went_high()) {
			// TODO: if ping_method != last_ping_method PingMethodAlgorithm::reset();
			auto newtime = PingMethodAlgorithm::filter(_ping_time, _ping_tmr, ping_method);
			if (newtime.has_value()) {
				_ping_time = newtime.value();
				_pingled_tmr = 0;

				_ping_changed = true;
				clk_out.high();
				bus_clk_out.high();

				// TODO: see if this reduces jitter by much:
				// controls.clk_out.high();
				// controls.ping_led.high();
				// if (!modes.ping_locked)
				// 	flags.set_time_changed();
			}
			_ping_tmr = 0;
		}

		if (_pingled_tmr >= _ping_time) {
			_pingled_tmr = 0;
			_ping_led_high = true;
			_ping_changed = true;
			clk_out.high();
			bus_clk_out.high();
		} else if (_pingled_tmr >= (_ping_time / 2)) {
			_ping_led_high = false;
			clk_out.low();
			bus_clk_out.low();
		}

		if (_loopled_tmr >= _loopled_time) { // && modes.inf == InfState::Off) {
			reset_loopled_tmr();
		} else if (_loopled_tmr >= (_loopled_time / 2)) {
			loop_led.low();
			loop_out.low();
		}
	}

	void set_divmult_time(uint32_t time) {
		_loopled_time = time;
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

	uint32_t reset_ping_tmr() {
		_ping_time = _ping_tmr;
		_ping_tmr_needs_reset = true;
		return _ping_time;
	}

	void reset_loopled_tmr() {
		loop_led.high();
		loop_out.high();
		_loopled_tmr = 0;
	}

	bool ping_led_high() {
		return _ping_led_high;
	}
};

} // namespace LDKit
