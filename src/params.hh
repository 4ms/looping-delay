#pragma once
#include "audio_stream_conf.hh"
#include "controls.hh"
#include "exp_1voct_lut.hh"
#include "util/countzip.hh"
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
	bool time_jack_quantized = true;
	bool ping_locked = false;
	bool quantize_mode_changes = true;
	bool adjust_loop_end = false; // flag_pot_changed_revdown[TIME]
	float scroll_loop_amount = 0;
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

	uint32_t crossfade_samples = 192;							   // SLOW_FADE_SAMPLES
	float crossfade_rate = calc_fade_increment(crossfade_samples); // SLOW_FADE_INCREMENT

	static constexpr float calc_fade_increment(uint32_t samples) {
		return (1.f / (((float)samples / (float)AudioStreamConf::BlockSize) + 1.f));
	}
};

enum class OperationMode { Normal, SysSettings, Calibrate };

// Params holds all the modes, settings and parameters for the looping delay
// Params are set by controls (knobs, jacks, buttons, etc)
struct Params {
	Controls &controls;

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
	uint32_t mute_on_boot_ctr = 12000;

	Params(Controls &controls)
		: controls{controls} {}

	void update() {
		controls.update();

		update_pot_states();
		update_adjust_loop_end_mode();
		update_time_quant_mode();

		update_cv_states();

		if (modes.inf == InfState::On) {
			delay_feed = 0.f;
			feedback = 1.f;
			update_scroll_loop_amount();
		} else {
			float df = cv_state[DelayFeedCV].cur_val + pot_state[DelayFeedPot].cur_val;
			delay_feed = std::clamp(df / 4095.f, 0.f, 4095.f);

			float fb = cv_state[FeedbackCV].cur_val + pot_state[FeedbackPot].cur_val;
			feedback = std::clamp(fb / 4095.f, 0.f, 4095.f);
		}

		int16_t time_cv = 2048 - cv_state[TimeCV].cur_val;
		int16_t time_pot = pot_state[TimePot].cur_val;
		float time_mult;
		if (modes.time_jack_quantized && modes.time_pot_quantized) {
			time_mult = calc_clk_div_quantized(time_pot) * calc_clk_div_quantized(time_cv);
		} else if (!modes.time_jack_quantized && !modes.time_pot_quantized) {
			if (time_cv > 30) {
				// Positive voltage on jack
				float comp_cv = (float)time_cv * tracking_comp;
				// TODO: interpolate lookup
				time_mult = calc_clk_div_unquant(time_pot / exp_1voct[(unsigned)comp_cv]);
			} else if (time_cv < -30) {
				// negative boltage on jack
			}
			time_mult = calc_clk_div_quantized(time_pot) * calc_clk_div_quantized(time_cv);
		}

		float mx = (controls.read_adc(MixCV) - 2048) + pot_state[MixPot].cur_val;
		// TODO; use epp lut
		mix_dry = std::clamp(mx / 4095.f, 0.f, 4095.f);
		mix_wet = 1.f - mix_dry;

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

		if (mute_on_boot_ctr)
			mute_on_boot_ctr--;
	}

	void reset_loopled_tmr() {}

	void update_pot_states() {
		for (auto [i, pot] : enumerate(pot_state)) {
			pot.cur_val = (int16_t)controls.read_adc(static_cast<AdcElement>(i++));

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

			if (controls.reverse_button.is_just_released()) {
				if (!ignore_rev_release) {
					// TODO: handle REV released
				}
				pot.moved_while_rev_down = false;
			}

			if (!controls.hold_button.is_pressed()) {
				if (!ignore_inf_release) {
					// TODO: handle INF released
				}
				pot.moved_while_inf_down = false;
			}
		}
	}

	void update_cv_states() {
		for (auto [i, cv] : enumerate(cv_state)) {
			cv.cur_val = (int16_t)controls.read_adc(static_cast<AdcElement>(NumPots + i++));
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
			modes.time_jack_quantized = false;
		} else if (pot_state[TimePot].moved) {
			modes.time_pot_quantized = true;
			if (!settings.auto_unquantize_timejack)
				modes.time_jack_quantized = true;
		}

		if (settings.auto_unquantize_timejack) {
			// Auto Unquantize mode: set jack to Quant when time is > 47Hz (48000 / 1024 = 47Hz)
			modes.time_jack_quantized = (divmult_time >= 1024);
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
			modes.scroll_loop_amount += (float)pot_state[FeedbackPot].delta /* + cv_delta*/ / 4096.f;
			pot_state[FeedbackPot].moved_while_inf_down = false;
		}
	}

	static constexpr float DivKnobValue[17] = {
		1.f, 1.5f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f};
	static constexpr int16_t DivKnobDetents[18] = {
		0, 40, 176, 471, 780, 1076, 1368, 1664, 1925, 2179, 2448, 2714, 2991, 3276, 3586, 3879, 4046, 4095};

	static constexpr float calc_clk_div_quantized(int16_t adc_val) {
		bool divide = false;
		float val = DivKnobValue[16];

		if (adc_val < 0) {
			adc_val = -adc_val;
			divide = true;
		}
		val = DivKnobValue[find_range_idx(adc_val)];
		return divide ? (1.f / val) : val;
	}

	static constexpr float calc_clk_div_unquant(int16_t adc_val) {
		bool divide = false;
		if (adc_val < 0) {
			adc_val = -adc_val;
			divide = true;
		}

		float val;
		unsigned range_i = find_range_idx(adc_val);
		if (range_i == 0)
			val = DivKnobValue[0];
		else {
			float floor = DivKnobValue[range_i - 1];
			float ceil = DivKnobValue[range_i];
			val = MathTools::map_value(adc_val, DivKnobDetents[range_i], DivKnobDetents[range_i + 1], floor, ceil);
		}
		return divide ? (1.f / val) : val;
	}

	static constexpr unsigned find_range_idx(uint16_t adc_val) {
		static_assert(DivKnobDetents[0] == 0, "");
		unsigned i;
		for (i = 1; i < 17; i++) {
			if (adc_val <= DivKnobDetents[i])
				break;
		}
		return i - 1;
	}

	static constexpr float adjust_time_by_switch(float timeval, uint8_t switch_val) {
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
	// int32_t window_pot_delta = 0;
	// int32_t window_cv_delta = 0;
};

constexpr auto ParamsSize = sizeof(Params); // 164B

static_assert(Params::calc_clk_div_quantized(0) == 1.0f, "");
static_assert(Params::calc_clk_div_quantized(39) == 1.0f, "");
static_assert(Params::calc_clk_div_quantized(40) == 1.0f, "");
static_assert(Params::calc_clk_div_quantized(41) == 1.5f, "");
static_assert(Params::calc_clk_div_quantized(4045) == 15.f, "");
static_assert(Params::calc_clk_div_quantized(4046) == 15.f, "");
static_assert(Params::calc_clk_div_quantized(4047) == 16.f, "");
static_assert(Params::calc_clk_div_quantized(4095) == 16.f, "");

static_assert(Params::find_range_idx(39) == 0, "");
static_assert(Params::find_range_idx(40) == 0, "");
static_assert(Params::find_range_idx(41) == 1, "");

static_assert(Params::calc_clk_div_unquant(39) == 1.0f, "");
static_assert(Params::calc_clk_div_unquant(40) == 1.000f, "");
static_assert(Params::calc_clk_div_unquant(41) >= 1.003f, "");
static_assert(Params::calc_clk_div_unquant(41) <= 1.004f, "");
static_assert(Params::calc_clk_div_unquant(42) >= 1.007f, "");
static_assert(Params::calc_clk_div_unquant(42) <= 1.008f, "");
static_assert(Params::calc_clk_div_unquant(780) == 3.0f, "");
static_assert(Params::calc_clk_div_unquant(928) == 3.5f, "");

static_assert(Params::calc_clk_div_unquant(4046) == 15.000f, "");

static_assert(Params::calc_clk_div_unquant(4047) >= 15.020f, "");
static_assert(Params::calc_clk_div_unquant(4047) <= 15.021f, "");

static_assert(Params::calc_clk_div_unquant(4048) >= 15.040f, "");
static_assert(Params::calc_clk_div_unquant(4048) <= 15.041f, "");

static_assert(Params::calc_clk_div_unquant(4094) >= 15.979f, "");
static_assert(Params::calc_clk_div_unquant(4094) <= 15.980f, "");

static_assert(Params::calc_clk_div_unquant(4095) == 16.f, "");

} // namespace LDKit
