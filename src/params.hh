#pragma once
#include "controls.hh"
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
	bool time_pot_quantized = false;
	bool time_jack_quantized = false;
	GateType loop_clock = GateType::Trig;
	GateType main_clock = GateType::Gate;
	bool pot_window_mode = false;
	bool jack_window_mode = false;
	bool levelcv_mix = false;
	bool ping_locked = false;
	bool send_return_before_loop = false;
};

struct Settings {
	bool auto_mute = true;
	bool soft_clip = true;
	bool dc_input = false;
	bool calibrate = false;
	bool system_settings = false;
	PingMethod ping_method = PingMethod::IGNORE_FLAT_DEVIATION_10;
	GateType rev_jack;
	GateType inf_jack;
	bool log_delay_feed = true;
	bool runaway_dc_block = true;
	bool quantize_mode_changes = true;
	uint32_t led_brightness = 4;

	float crossfade_rate = 0.01f;
};

inline constexpr float DivKnobValue[17] = {
	1.f, 1.5f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f};

// Params holds all the modes, settings and parameters for the looping delay
// Params are set by controls (knobs, jacks, buttons, etc)
struct Params {
	Controls &controls;

	float time; // TIME: fractional value for time multiplication, integer value for time division
	float divmult_time;
	float delay_feed; // DELAY FEED: amount of main input mixed into delay loop
	float feedback;	  // FEEDBACK: amount of regeneration
	float mix_dry;	  // MIX: mix of delayed and clean on the main output
	float mix_wet;
	float tracking_comp; // TRACKING_COMP: -2.0 .. 2.0 compensation for 1V/oct tracking

	ChannelMode modes;
	Settings settings;

	float fast_fade_samples;
	float slow_fade_samples;
	float fast_fade_increment;

	Params(Controls &controls)
		: controls{controls} {}

	void update() {
		controls.update();
		// // LPF?
		float df = controls.read_adc(DelayFeedCV) + controls.read_adc(DelayFeedPot);
		delay_feed = std::clamp(df / 4095.f, 0.f, 4095.f);
	}

	// float set_fade_increment(uint32_t samples) { return (1.f / ((samples / (codec_BUFF_LEN >> 3)) + 1.f)); }
};

} // namespace LDKit
