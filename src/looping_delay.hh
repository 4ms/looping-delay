#pragma once
#include "audio_stream_conf.hh"
#include "controls.hh"
#include "debug.hh"
#include "delay_buffer.hh"
#include "params.hh"
#include "util/math.hh"
#include "util/zip.hh"

namespace LDKit
{

class LoopingDelay {
	Params &params;
	DelayBuffer &buf;

	uint32_t read_head;
	uint32_t write_head;
	uint32_t fade_read_head;
	uint32_t fade_read_head_end;
	uint32_t fade_queued_divmult_time_end;

	uint32_t loop_start;
	uint32_t loop_end;

public:
	LoopingDelay(Params &params, DelayBuffer &delay_buffer)
		: params{params}
		, buf{delay_buffer} {
		// for (auto &s : buf)
		// 	s = 0;
		//
	}

	void update(const AudioStreamConf::AudioInBlock inblock, AudioStreamConf::AudioOutBlock outblock) {
		check_heads_in_bounds();

		Debug::Pin3::high();
		for (auto [out, in] : zip(outblock, inblock)) {
			out.chan[0] = in.chan[0] * params.delay_feed;
			out.chan[1] = in.chan[1];
		}
		Debug::Pin3::low();
	}

	void check_heads_in_bounds() {
		if (params.modes.inf == InfState::Off && fade_read_head < params.settings.crossfade_rate) {
			// auto t = calculate_read_addr(params.divmult_time);
		}
	}
};

// TODO: would be nice to give SAI LoopingDelay::update directly
// IRQ -> ISRs[n]
// -> if saiflag { callback(0); } else { callback(1); }
// -> AudioStream::_process_func(buf[0/1])
// -> LoopingDelay::update(bufs)

} // namespace LDKit
