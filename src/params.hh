#pragma once
#include "controls.hh"
#include <cstdint>

namespace LDKit
{

struct Params {
	Controls &controls;
	Params(Controls &controls)
		: controls{controls} {}

	void update() {
		controls.update();
		// LPF?
		float delay_feed = controls.read_adc(DelayFeedCV) + controls.read_adc(DelayFeedPot);
		level = std::clamp(delay_feed / 4095.f, 0.f, 4095.f);
	}

	float time;	   // TIME: fractional value for time multiplication, integer value for time division
	float level;   // LEVEL: 0..1 amount of main input mixed into delay loop
	float regen;   // REGEN: 0..1 amount of regeneration
	float mix_dry; // MIX: 0..1 mix of delayed and clean on the main output
	float mix_wet;
	float tracking_comp; // TRACKING_COMP: -2.0 .. 2.0 compensation for 1V/oct tracking

	enum class GateType { Gate, Trig };

	struct ChannelMode {
		enum class InfState { Off, On, TransitioningOn, TransitioningOff };
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
	} mode;

	struct GlobalMode {
		bool auto_mute = true;
		bool soft_clip = true;
		bool dc_input = false;
		bool calibrate = false;
		bool system_settings = false;
		GateType rev_jack;
		GateType inf_jack;
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
		PingMethod ping_method;
		bool log_delay_feed = true;
		bool runaway_dc_block = true;
		bool quantize_mode_changes = true;
	} system_mode;

	float fast_fade_samples;
	float slow_fade_samples;
	float fast_fade_increment;
	float slow_fade_increment;

	uint32_t loop_led_brightness = 4;

	static constexpr float DivKnobValue[17] = {
		1.f,
		1.5f,
		2.f,
		3.f,
		4.f,
		5.f,
		6.f,
		7.f,
		8.f,
		9.f,
		10.f,
		11.f,
		12.f,
		13.f,
		14.f,
		15.f,
		16.f,
	};
};

} // namespace LDKit
