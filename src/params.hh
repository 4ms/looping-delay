#pragma once
#include "audio_stream_conf.hh"
#include "clock_mult_util.hh"
#include "controls.hh"
#include "epp_lut.hh"
#include "flags.hh"
#include "leds.hh"
#include "log_taper_lut.hh"
#include "modes.hh"
#include "trig_ins.hh"
#include "util/countzip.hh"
#include "util/math.hh"
#include <cstdint>

namespace LDKit
{
// Params holds all the modes, settings and parameters for the looping delay
// Params are set by controls (knobs, jacks, buttons, etc)
struct Params {
	Controls &controls;
	Flags &flags;
	TrigIns trig_ins;

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
	Settings settings;
	OperationMode op_mode = OperationMode::Normal;
	Leds leds{controls, modes};

	Params(Controls &controls, Flags &flags, Timer &timer)
		: controls{controls}
		, flags{flags}
		, trig_ins{controls, modes, flags, timer} {}

	void update() {
		controls.update();
		ping_time = trig_ins.update_ping(ping_time);

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

		leds.update();

		update_button_modes();

		if (flags.mute_on_boot_ctr)
			flags.mute_on_boot_ctr--;
	}

	void process_mode_flags() {
		// all handled in looping delay and flags
	}

	void reset_loopled_tmr() {}

	// TODO: to use a double-buffer params, then
	// looping delay should set a flag that tells params to set a
	// new state for these
	void set_inf_state(InfState newstate) { modes.inf = newstate; }
	void toggle_reverse() { modes.reverse = !modes.reverse; }
	void set_divmult(float new_divmult) { divmult_time = new_divmult; }

private:
	void update_pot_states() {
		for (auto [i, pot] : enumerate(pot_state)) {
			pot.cur_val = (int16_t)controls.read_pot(static_cast<PotAdcElement>(i++));

			int16_t diff = std::abs(pot.cur_val - pot.prev_val);
			if (diff > Board::MinPotChange)
				pot.track_moving_ctr = 250;

			if (pot.track_moving_ctr) {
				pot.track_moving_ctr--;
				pot.prev_val = pot.cur_val;
				pot.delta = diff;
				pot.moved = true;

				if (controls.reverse_button.is_pressed()) {
					pot.moved_while_rev_down = true;
					ignore_rev_release = true; // if i==TimePot and in InfMode only?
				}

				if (controls.reverse_button.is_pressed()) {
					pot.moved_while_inf_down = true;
					ignore_inf_release = true; // if i==TimePot || FeedbackPot in InfMode only?
				}
			}
		}
	}

	void update_cv_states() {
		for (auto [i, cv] : enumerate(cv_state)) {
			cv.cur_val = (int16_t)controls.read_cv(static_cast<CVAdcElement>(i++));
			if (op_mode == OperationMode::Calibrate) {
				// TODO: use raw values, without calibration offset
			}
			int16_t diff = std::abs(cv.cur_val - cv.prev_val);
			if (diff > Board::MinCVChange) {
				cv.delta = diff;
				cv.prev_val = cv.cur_val;
			}
		}
	}

	void update_button_modes() {
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
			// Auto Unquantize mode: set jack to Quant when time is > 47Hz (48000 / 1024 = 47Hz)
			modes.time_cv_quantized = (divmult_time >= 1024);
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
			flags.add_scroll_amt((float)pot_state[FeedbackPot].delta /* + cv_delta*/ / 4096.f);
			pot_state[FeedbackPot].moved_while_inf_down = false;
		}
	}

	void calc_delay_feed() {
		if (modes.inf == InfState::On) {
			delay_feed = 0.f;
			return;
		}

		int16_t df = pot_state[DelayFeedPot].cur_val;
		if (!settings.levelcv_mix) {
			df = __USAT(df + cv_state[DelayFeedCV].cur_val, 12);
		}

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
		uint16_t mx;
		if (settings.levelcv_mix)
			mx = __USAT(cv_state[MixCV].cur_val + pot_state[MixPot].cur_val, 12);
		else
			mx = pot_state[MixPot].cur_val;

		mix_dry = epp_lut[mx];
		mix_wet = epp_lut[4095 - mx];
	}

	// void update_button_modes() {
	// }

	static constexpr float adjust_time_by_switch(float timeval, Controls::SwitchPos switch_pos) {
		uint16_t switch_val = static_cast<uint16_t>(switch_pos);
		if (switch_val == 0b10)
			return timeval + 16.f; // switch up: 17-32
		if (switch_val == 0b01)
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
