#pragma once
#include "audio_stream_conf.hh"
#include "clock_mult_util.hh"
#include "controls.hh"
#include "epp_lut.hh"
#include "flags.hh"
#include "log_taper_lut.hh"
#include "util/countzip.hh"
#include "util/math.hh"
#include <cstdint>

namespace LDKit
{
enum class GateType { Gate, Trig };
enum class InfState { Off, On, TransitioningOn, TransitioningOff };
enum class PingMethod {
	IGNORE_FLAT_DEVIATION_10,
	IGNORE_PERCENT_DEVIATION,
	ONE_TO_ONE,
	MOVING_AVERAGE_2,
	LINEAR_AVERAGE_4,
	EXPO_AVERAGE_8,
	IGNORE_FLAT_DEVIATION_5,
	MOVING_AVERAGE_4,
	EXPO_AVERAGE_4,
	LINEAR_AVERAGE_8,
};

struct ChannelMode {
	InfState inf = InfState::Off;
	bool reverse = false;
	bool time_pot_quantized = true;
	bool time_cv_quantized = true;
	bool ping_locked = false;
	bool quantize_mode_changes = true;
	bool adjust_loop_end = false; // flag_pot_changed_revdown[TIME]
};

// Settings cannot be changed in Normal operation mode
struct Settings {
	bool auto_mute = true;
	bool soft_clip = true;
	bool dc_input = false;
	PingMethod ping_method = PingMethod::IGNORE_FLAT_DEVIATION_10;
	GateType rev_jack = GateType::Trig;
	GateType inf_jack = GateType::Trig;
	GateType loop_clock = GateType::Trig;
	GateType main_clock = GateType::Gate;
	bool log_delay_feed = true;
	bool runaway_dc_block = true;
	bool auto_unquantize_timejack = true;
	bool send_return_before_loop = false;
	uint32_t led_brightness = 4;
	bool levelcv_mix = false;

	uint32_t crossfade_samples = 192;									 // SLOW_FADE_SAMPLES
	float crossfade_rate = calc_fade_increment(crossfade_samples);		 // SLOW_FADE_INCREMENT
	uint32_t write_crossfade_samples = 192;								 // FAST_FADE_SAMPLES
	float write_crossfade_rate = calc_fade_increment(crossfade_samples); // FAST_FADE_INCREMENT

	static constexpr float calc_fade_increment(uint32_t samples) {
		return (1.f / (((float)samples / (float)AudioStreamConf::BlockSize) + 1.f));
	}
};

enum class OperationMode { Normal, SysSettings, Calibrate };

// Params holds all the modes, settings and parameters for the looping delay
// Params are set by controls (knobs, jacks, buttons, etc)
struct Params {
	Controls &controls;
	Flags &flags;

	float time = 0.f;		 // TIME: fractional value for time multiplication, integer value for time division
	float delay_feed = 0.7f; // DELAY FEED: amount of main input mixed into delay loop
	float feedback = 0.5f;	 // FEEDBACK: amount of regeneration
	float mix_dry = 0.7f;	 // MIX: mix of delayed and clean on the main output
	float mix_wet = 0.7f;
	float tracking_comp = 1.f; // TRACKING_COMP: -2.0 .. 2.0 compensation for 1V/oct tracking
	float divmult_time;		   // samples between read and write heads
	uint32_t ping_time;
	uint32_t locked_ping_time;

	ChannelMode modes;
	Settings settings;
	OperationMode op_mode = OperationMode::Normal;

	Params(Controls &controls, Flags &flags)
		: controls{controls}
		, flags{flags} {}

	void update() {
		controls.update();

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
			//  update_calibration_leds();
		}

		if (op_mode == OperationMode::SysSettings) {
			// TODO: System Settings mode
			//  update_system_settings();
			//  update_system_settings_leds();
		}

		// check_entering_system_mode();

		// update_leds();

		update_button_modes();

		if (flags.mute_on_boot_ctr)
			flags.mute_on_boot_ctr--;
	}

	void process_mode_flags() {
		// if (!disable_mode_changes) {
		// 	if (flag_inf_change[channel]) {
		// 		change_inf_mode(channel);
		// 	}

		// 	if (flag_rev_change[channel]) {
		// 		// mode[channel][CONTINUOUS_REVERSE] = 0;

		// 		if (!doing_reverse_fade[channel]) {
		// 			flag_rev_change[channel] = 0;

		// 			mode[channel][REV] = 1 - mode[channel][REV];

		// 			if (mode[channel][INF] == INF_ON || mode[channel][INF] == INF_TRANSITIONING_OFF ||
		// 				mode[channel][INF] == INF_TRANSITIONING_ON)
		// 				reverse_loop(channel);

		// 			else
		// 				swap_read_write(channel);
		// 		}
		// 	}
		// }

		// handled in looping delay:
		// if (flag_time_changed || flag_ping_changed) {
		// 	flag_time_changed = false;
		// 	flag_ping_changed = false;
		// 	set_divmult_time();
		// }
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

			if (!controls.hold_button.is_pressed()) {
				if (!ignore_inf_release) {
					// TODO: handle INF released
				}
				pot.moved_while_inf_down = false;
			}
		}
	}

	void update_button_modes() {
		if (controls.hold_button.is_just_released()) {
			if (!ignore_inf_release) {
				flags.set_inf_changed();
			}

			ignore_rev_release = false;
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

	void update_cv_states() {
		for (auto [i, cv] : enumerate(cv_state)) {
			cv.cur_val = (int16_t)controls.read_cv(static_cast<CVAdcElement>(NumPots + i++));
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
		if (!settings.levelcv_mix)
			df = __USAT(df + cv_state[DelayFeedCV].cur_val, 12);

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

constexpr auto ParamsSize = sizeof(Params); // 164B

} // namespace LDKit
