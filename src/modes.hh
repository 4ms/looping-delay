#pragma once
#include "audio_stream_conf.hh"
#include <cstdint>

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
