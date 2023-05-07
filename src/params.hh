#pragma once
#include "audio_stream_conf.hh"
#include "calibration_storage.hh"
#include "clock_mult_util.hh"
#include "controls.hh"
#include "epp_lut.hh"
#include "flags.hh"
#include "log_taper_lut.hh"
#include "modes.hh"
#include "timer.hh"
#include "util/countzip.hh"
#include "util/math.hh"

namespace LDKit
{
// Params holds all the modes, settings and parameters for the looping delay
// Params are set by controls (knobs, jacks, buttons, etc)
struct Params {
	Controls &controls;
	Flags &flags;
	Timer &timer;

	// TODO: double-buffer Params:
	// put just these into its own struct
	float time = 0.f;		 // TIME: fractional value for time multiplication, integer value for time division
	float delay_feed = 0.7f; // DELAY FEED: amount of main input mixed into delay loop
	float feedback = 0.5f;	 // FEEDBACK: amount of regeneration
	float mix_dry = 0.7f;	 // MIX: mix of delayed and clean on the main output
	float mix_wet = 0.7f;
	float tracking_comp = 1.f;	// TRACKING_COMP: -2.0 .. 2.0 compensation for 1V/oct tracking
	float divmult_time = 12000; // samples between read and write heads
	uint32_t ping_time = 12000;
	uint32_t locked_ping_time = 12000;

	ChannelMode modes;
	OperationMode op_mode = OperationMode::Normal;

	CalibrationStorage cal_storage;
	CalibrationData &calibration = cal_storage.cal_data;
	Settings &settings = calibration.settings;

	Params(Controls &controls, Flags &flags, Timer &timer)
		: controls{controls}
		, flags{flags}
		, timer{timer} {}

	void update() {
		controls.update();

		update_trig_jacks();

		update_pot_states();
		update_cv_states();

		update_adjust_loop_end_mode();
		update_time_quant_mode();

		calc_delay_feed();
		calc_feedback();
		calc_time();
		calc_mix();

		if (modes.inf == InfState::On)
			update_scroll_loop_amount();

		// TODO
		tracking_comp = 1.f;

		if (op_mode == OperationMode::Calibrate) {
			// TODO: Calibrate mode
			//  update_calibration();
		}

		if (op_mode == OperationMode::SysSettings) {
			// TODO: System Settings mode
			//  update_system_settings();
		}

		// check_entering_system_mode();

		update_leds();
		update_button_modes();

		if (flags.mute_on_boot_ctr)
			flags.mute_on_boot_ctr--;
	}

	void reset_loop() {
		controls.loop_led.high();
		controls.loop_out.high();
		timer.reset_loopled_tmr();
	}

	// TODO: to use a double-buffer params, then
	// looping delay should set a flag that tells params to set a
	// new state for these
	void set_inf_state(InfState newstate) { modes.inf = newstate; }
	void toggle_reverse() { modes.reverse = !modes.reverse; }
	void set_divmult(float new_divmult) { divmult_time = new_divmult; }

private:
	void update_trig_jacks() {
		if (timer.take_ping_changed()) {
			controls.clk_out.high();
			controls.bus_clk_out.high();
			controls.ping_led.high();
			ping_time = timer.get_ping_time();
			if (!modes.ping_locked)
				flags.set_time_changed();
		}

		if (controls.reverse_jack.is_just_pressed()) {
			flags.set_rev_changed();
		}
		if (controls.inf_jack.is_just_pressed()) {
			flags.set_inf_changed();
		}
	}

	void update_pot_states() {
		for (auto [i, pot] : enumerate(pot_state)) {
			pot.cur_val = (int16_t)controls.read_pot(static_cast<PotAdcElement>(i++));

			int16_t diff = pot.cur_val - pot.prev_val;
			if (std::abs(diff) > Brain::MinPotChange)
				pot.track_moving_ctr = 10; // track for at least 6ms

			if (pot.track_moving_ctr) {
				pot.track_moving_ctr--;
				pot.prev_val = pot.cur_val;
				pot.delta = diff;
				pot.moved = true;

				if (controls.reverse_button.is_pressed()) {
					pot.moved_while_rev_down = true;
					ignore_rev_release = true; // if i==TimePot and in InfMode only?
				}

				if (controls.inf_button.is_pressed()) {
					pot.moved_while_inf_down = true;
					ignore_inf_release = true; // if i==TimePot || FeedbackPot in InfMode only?
				}
			} else {
				pot.moved = false;
				pot.delta = 0;
			}
		}
	}

	void update_cv_states() {
		for (auto [i, cv] : enumerate(cv_state)) {
			cv.cur_val = (int16_t)controls.read_cv(static_cast<CVAdcElement>(i++));
			if (op_mode == OperationMode::Calibrate) {
				// TODO: use raw values, without calibration offset
			}
			int16_t diff = cv.cur_val - cv.prev_val;
			if (std::abs(diff) > Brain::MinCVChange) {
				cv.delta = diff;
				cv.prev_val = cv.cur_val;
			}
		}
	}

	void update_button_modes() {
		if (controls.ping_button.just_went_low()) {
			// TODO: handle entering modes: QMC, Ping Locked
			// if (!INF1BUT && !INF2BUT && REV1BUT && REV2BUT) {
			// 	flag_acknowlegde_qcm = (6 << 8);

			// 	if (global_mode[QUANTIZE_MODE_CHANGES] == 0)
			// 		global_mode[QUANTIZE_MODE_CHANGES] = 1;
			// 	else
			// 		global_mode[QUANTIZE_MODE_CHANGES] = 0;

			// 	flag_ignore_revdown[0] = 1;
			// 	flag_ignore_revdown[1] = 1;
			// } else if (REV1BUT && !INF1BUT && !INF2BUT && !REV2BUT) {
			// 	flag_ignore_revdown[0] = 1;
			// } else if (REV2BUT && !INF1BUT && !INF2BUT && !REV1BUT) {
			// 	flag_ignore_revdown[1] = 1;
			// }

			// else if (INF1BUT && !INF2BUT && !REV1BUT && !REV2BUT)
			// {
			// 	if (mode[0][PING_LOCKED] == 0) {
			// 		locked_ping_time[0] = ping_time;
			// 		mode[0][PING_LOCKED] = 1;
			// 	} else {
			// 		mode[0][PING_LOCKED] = 0;
			// 		set_divmult_time(0);
			// 	}

			// 	flag_ignore_infdown[0] = 1;

			// } else if (INF2BUT && INF1BUT && REV1BUT && REV2BUT) {
			// 	flag_ignore_revdown[0] = 1;
			// 	flag_ignore_revdown[1] = 1;
			// 	flag_ignore_infdown[0] = 1;
			// 	flag_ignore_infdown[1] = 1;

			// } else if (!INF2BUT && !INF1BUT && !REV1BUT && !REV2BUT) {
			ping_time = timer.get_ping_tmr();
			controls.clk_out.high();
			controls.bus_clk_out.high();
			timer.reset_ping_tmr();
			timer.reset_pingled_tmr();
			// timer.reset_clkout_tmr();

			// TODO: this is handled automatically now, right?
			if (modes.quantize_mode_changes) {
				// 	process_mode_flags();
			}
			if (!modes.ping_locked)
				flags.set_time_changed();
		}

		if (controls.inf_button.is_just_released()) {
			if (!ignore_inf_release) {
				flags.set_inf_changed();
			}

			ignore_inf_release = false;
			for (auto &pot : pot_state)
				pot.moved_while_inf_down = false;
		}

		if (controls.reverse_button.is_just_released()) {
			if (!ignore_rev_release) {
				flags.set_rev_changed();
			}

			ignore_rev_release = false;
			for (auto &pot : pot_state)
				pot.moved_while_rev_down = false;
		}
	}

	void update_leds() {
		if (modes.inf == InfState::TransitioningOn)
			controls.inf_led.high();
		else if (modes.inf == InfState::TransitioningOff)
			controls.inf_led.low();

		controls.reverse_led.set(modes.reverse);

		// if (flag_acknowlegde_qcm) {
		// 	flag_acknowlegde_qcm--;
		// 	if ((flag_acknowlegde_qcm & (1 << 8)) || (!global_mode[QUANTIZE_MODE_CHANGES] && (flag_acknowlegde_qcm & (1
		// << 6))))
		// 	{
		// 		LED_PINGBUT_ON;
		// 		LED_REV1_ON;
		// 		LED_REV2_ON;
		// 	} else {
		// 		LED_PINGBUT_OFF;
		// 		LED_REV1_OFF;
		// 		LED_REV2_OFF;
		// 	}
		// }

		if (controls.ping_button.is_pressed()) {
			controls.ping_led.high();
		}

		auto ping_ledbut_tmr = timer.get_pingled_tmr();
		if (ping_ledbut_tmr >= ping_time) {
			controls.ping_led.high();
			timer.reset_pingled_tmr();
			controls.clk_out.high();
			controls.bus_clk_out.high();
		} else if (ping_ledbut_tmr >= (ping_time / 2)) {
			if (!controls.ping_button.is_pressed())
				controls.ping_led.low();
			controls.clk_out.low();
			controls.bus_clk_out.low();
		}

		auto loopled_tmr = timer.get_loopled_tmr();
		if (loopled_tmr >= divmult_time && modes.inf == InfState::Off) {
			reset_loop();
		} else if (loopled_tmr >= (divmult_time / 2)) {
			controls.loop_led.low();
			controls.loop_out.low();
		}
	}

	void update_time_quant_mode() {
		// Holding INF while turning Time changes to NoQuant mode
		// Turning Time without INF down changes it back to Quant mode (default)
		if (pot_state[TimePot].moved_while_inf_down) {
			modes.time_pot_quantized = false;
			modes.time_cv_quantized = false;
		} else if (pot_state[TimePot].moved) {
			modes.time_pot_quantized = true;
			if (!settings.auto_unquantize_timejack)
				modes.time_cv_quantized = true;
		}

		if (settings.auto_unquantize_timejack) {
			// Auto Unquantize mode: set jack to Quant when time is > 23Hz (48000 / 2048 = 23Hz)
			modes.time_cv_quantized = (divmult_time >= 2048);
		}
	}

	void update_adjust_loop_end_mode() {
		// In INF mode, holding REV and turning Time adjusts the loop end
		if (pot_state[TimePot].moved_while_rev_down && modes.inf == InfState::On)
			modes.adjust_loop_end = true;
		else
			modes.adjust_loop_end = false;
	}

	void update_scroll_loop_amount() {
		if (pot_state[FeedbackPot].moved_while_inf_down) {
			float scroll_amt = (float)pot_state[FeedbackPot].delta / 4096.f;
			flags.add_scroll_amt(scroll_amt);
			pot_state[FeedbackPot].moved_while_inf_down = false;
		}
	}

	void calc_delay_feed() {
		if (modes.inf == InfState::On) {
			delay_feed = 0.f;
			return;
		}

		uint16_t df = __USAT(pot_state[DelayFeedPot].cur_val + cv_state[DelayFeedCV].cur_val, 12);

		if (settings.log_delay_feed)
			delay_feed = log_taper[df];
		else
			delay_feed = df > 4065 ? 1.f : (float)MathTools::plateau<30, 0>(df) / 4035.f;
	}

	void calc_feedback() {
		if (modes.inf == InfState::On) {
			feedback = 1.f;
			return;
		}

		float fb_pot = pot_state[FeedbackPot].cur_val;
		float fb;
		if (fb_pot < 3500.f)
			fb = fb_pot / 3500.f;
		else if (fb_pot < 4000.f)
			fb = 1.f;
		else
			fb = (fb_pot - 3050.f) / 950.f; //(4095-3050)/950 = 110% ... (4000-3050)/950 = 100%

		float fb_cv = cv_state[FeedbackCV].cur_val;
		// FIXME: DLD firmware has bug that prevents the fb += 1.f branch
		if (fb_cv > 4080.f)
			fb += 1.f;
		else if (fb_cv > 30.f)
			fb += fb_cv / 4095.f;

		if (fb > 1.1f)
			fb = 1.1f;
		else if (fb > 0.997f && fb < 1.003f)
			fb = 1.0f;

		feedback = fb;
	}

	void calc_time() {
		int16_t time_pot = pot_state[TimePot].cur_val;
		int16_t time_cv = MathTools::plateau<60, 0>(2048 - cv_state[TimeCV].cur_val);

		float time_pot_mult =
			modes.time_pot_quantized ? ClockMultUtil::calc_quantized(time_pot) : ClockMultUtil::calc_unquant(time_pot);

		float time_cv_mult = modes.time_cv_quantized ? ClockMultUtil::calc_quantized(time_cv) :
							 (time_cv < 0)			 ? ClockMultUtil::calc_unquant(time_cv) :
													   ClockMultUtil::calc_voct(time_cv, tracking_comp);

		auto time_switch = controls.read_time_switch();
		float time_mult = adjust_time_by_switch(time_pot_mult * time_cv_mult, time_switch);

		if (time != time_mult) {
			time = time_mult;
			flags.set_time_changed();
		}
	}

	void calc_mix() {
		uint16_t mx = __USAT(cv_state[MixCV].cur_val + pot_state[MixPot].cur_val, 12);

		mix_dry = epp_lut[mx];
		mix_wet = epp_lut[4095 - mx];
	}

	// void update_button_modes() {
	// }

	static constexpr float adjust_time_by_switch(float timeval, Controls::SwitchPos switch_pos) {
		if (switch_pos == Controls::SwitchPos::Up)
			return timeval + 16.f; // switch up: 17-32
		if (switch_pos == Controls::SwitchPos::Down)
			return timeval / 8.f; // switch down: eighth notes
		return timeval;
	}

private:
	struct PotState {
		int16_t cur_val = 0;
		int16_t prev_val = 0;			   // old_i_smoothed_potadc
		int16_t track_moving_ctr = 0;	   // track_moving_pot
		int16_t delta = 0;				   // pot_delta
		bool moved_while_inf_down = false; // flag_pot_changed_infdown
		bool moved_while_rev_down = false; // flag_pot_changed_revdown
		bool moved = false;				   // flag_pot_changed
	};
	std::array<PotState, NumPots> pot_state;

	struct CVState {
		int16_t cur_val = 0;
		int16_t prev_val = 0;
		int16_t delta = 0;
	};
	std::array<CVState, NumPots> cv_state;

	bool ignore_inf_release = false;
	bool ignore_rev_release = false;
};

constexpr auto ParamsSize = sizeof(Params);

} // namespace LDKit
