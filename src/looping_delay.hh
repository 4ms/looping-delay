#pragma once
#include "audio_stream_conf.hh"
#include "controls.hh"
#include "debug.hh"
#include "delay_buffer.hh"
#include "loop_util.hh"
#include "params.hh"
#include "util/math.hh"
#include "util/zip.hh"

namespace LDKit
{

class LoopingDelay {
	Params &params;
	DelayBuffer &buf;

	uint32_t read_head; // read_addr
	uint32_t write_head;
	float fade_read_phase;		 // read_fade_pos
	uint32_t fade_read_head_end; // fade_dest_read_addr
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

	void update(const AudioStreamConf::AudioInBlock &inblock, AudioStreamConf::AudioOutBlock &outblock) {
		Debug::Pin3::high();

		if (params.modes.inf != InfState::Off)
			check_read_head_in_loop();
		else
			check_read_write_head_spacing();

		for (auto [out, in] : zip(outblock, inblock)) {
			out.chan[0] = in.chan[0]; // * params.delay_feed;
			out.chan[1] = in.chan[1];
		}

		Debug::Pin3::low();
	}

	void check_read_head_in_loop() {
		// If we're not crossfading, check if the read head is inside the loop
		if (!Util::in_between(read_head, loop_start, loop_end, params.modes.reverse) && !is_crossfading()) {
			start_crossfade(loop_start);
			params.reset_loopled_tmr();
		}
	}

	void check_read_write_head_spacing() {
		if (!is_crossfading()) {
			auto t = calculate_read_addr(params.divmult_time);
			if (t != read_head)
				set_divmult_time();
		}
	}

	uint32_t calculate_read_addr(uint32_t new_divmult_time) {
		return Util::offset_samples(write_head, new_divmult_time, !params.modes.reverse);
	}

	void set_divmult_time() {
		uint32_t use_ping_time = params.modes.ping_locked ? params.locked_ping_time : params.ping_time;
		uint32_t t_divmult_time = use_ping_time * params.time;
		// t_divmult_time = t_divmult_time & 0xFFFFFFFC; //force it to be a multiple of 4

		// Check for valid divmult time range
		if (t_divmult_time > (Board::ExternalMemorySizeBytes / Board::RAMSampleSize))
			t_divmult_time = Board::ExternalMemorySizeBytes / Board::RAMSampleSize;

		if (params.modes.inf == InfState::Off) {
			if (is_crossfading()) {
				fade_queued_divmult_time_end = t_divmult_time;
			} else {
				params.divmult_time = t_divmult_time;
				start_crossfade(calculate_read_addr(params.divmult_time));
			}
		} else {
			if (params.divmult_time == t_divmult_time)
				return;

			params.divmult_time = t_divmult_time;

			if (params.pot_moved_while_rev_pressed[TimePot])
				loop_end = Util::offset_samples(loop_start, t_divmult_time, params.modes.reverse);
			else
				loop_start = Util::offset_samples(loop_end, t_divmult_time, 1 - params.modes.reverse);

			// If the read addr is not in between the loop start and end, then fade to the loop start
			if (!Util::in_between(read_head, loop_start, loop_end, params.modes.reverse)) {
				if (is_crossfading()) {
					fade_queued_divmult_time_end = t_divmult_time;
				} else {
					start_crossfade(loop_start);
					params.reset_loopled_tmr();
				}
			}
		}
	}

	bool is_crossfading() { return fade_read_phase >= params.settings.crossfade_rate; }

	void start_crossfade(uint32_t addr) {
		if (addr != read_head) {
			fade_read_phase = params.settings.crossfade_rate;
		}
		fade_queued_divmult_time_end = 0; // means: no queued crossfade
		fade_read_head_end = addr;
	}
};

// TODO: would be nice to give SAI LoopingDelay::update directly
// IRQ -> ISRs[n]
// -> if saiflag { callback(0); } else { callback(1); }
// -> AudioStream::_process_func(buf[0/1])
// -> LoopingDelay::update(bufs)

} // namespace LDKit
