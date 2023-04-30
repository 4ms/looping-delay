#pragma once
#include "audio_stream_conf.hh"
#include <cstdint>

enum class GateType { Gate = 0, Trig = 1 };
enum class InfState { Off, On, TransitioningOn, TransitioningOff };
enum class PingMethod : uint8_t {
	IGNORE_FLAT_DEVIATION_10 = 0,
	IGNORE_PERCENT_DEVIATION,
	ONE_TO_ONE,
	MOVING_AVERAGE_2,
	LINEAR_AVERAGE_4,
	EXPO_AVERAGE_8,
	IGNORE_FLAT_DEVIATION_5,
	MOVING_AVERAGE_4,
	EXPO_AVERAGE_4,
	LINEAR_AVERAGE_8,

	NUM_PING_METHODS,
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
	bool auto_mute;
	bool soft_clip;
	bool dc_input;
	PingMethod ping_method;
	GateType rev_jack;
	GateType inf_jack;
	GateType loop_clock;
	GateType main_clock;
	bool log_delay_feed;
	bool runaway_dc_block;
	bool auto_unquantize_timejack;
	bool send_return_before_loop;

	uint32_t crossfade_samples;		  // SLOW_FADE_SAMPLES
	float crossfade_rate;			  // SLOW_FADE_INCREMENT
	uint32_t write_crossfade_samples; // FAST_FADE_SAMPLES
	float write_crossfade_rate;		  // FAST_FADE_INCREMENT

	static constexpr float calc_fade_increment(uint32_t samples) {
		return (1.f / (((float)samples / (float)AudioStreamConf::BlockSize) + 1.f));
	}
};

enum class OperationMode { Normal, SysSettings, Calibrate };
