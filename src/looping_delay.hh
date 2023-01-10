#pragma once
#include "audio_stream_conf.hh"
#include "controls.hh"
#include "delay_buffer.hh"
#include "params.hh"
#include "util/math.hh"
#include "util/zip.hh"

namespace LDKit
{

class LoopingDelay {
	Params &params;
	DelayBuffer &buf;

public:
	LoopingDelay(Params &params, DelayBuffer &delay_buffer)
		: params{params}
		, buf{delay_buffer} {
		for (auto &s : buf)
			s = 0;
		//
	}

	void update(const AudioStreamConf::AudioInBlock inblock, AudioStreamConf::AudioOutBlock outblock) {
		float delay_feed = params.level;

		for (auto [out, in] : zip(outblock, inblock)) {
			out.chan[0] = in.chan[0] * delay_feed;
			out.chan[1] = in.chan[1];
		}
	}
};

// TODO: would be nice to give SAI LoopingDelay::update directly
// IRQ -> ISRs[n]
// -> if saiflag { callback(0); } else { callback(1); }
// -> AudioStream::_process_func(buf[0/1])
// -> LoopingDelay::update(bufs)

} // namespace LDKit
