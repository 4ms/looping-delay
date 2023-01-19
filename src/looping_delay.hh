#pragma once
#include "audio_memory.hh"
#include "audio_stream_conf.hh"
#include "auto_mute.hh"
#include "compress.hh"
#include "controls.hh"
#include "dcblock.hh"
#include "debug.hh"
#include "delay_buffer.hh"
#include "epp_lut.hh"
#include "loop_util.hh"
#include "params.hh"
#include "util/math.hh"
#include "util/zip.hh"
#include <algorithm>

namespace LDKit
{

class LoopingDelay {
	Params &params;
	DelayBuffer &buf;

	uint32_t read_head; // read_addr
	uint32_t write_head;
	float fade_read_phase;		  // read_fade_pos
	uint32_t fade_dest_read_head; // fade_dest_read_addr
	uint32_t fade_queued_divmult_time_end;

	uint32_t loop_start;
	uint32_t loop_end;

	bool doing_reverse_fade = false;

	AutoMute<0.0002f, 10, 0.02f, 0.02f> main_automute;
	AutoMute<0.0002f, 10, 0.02f, 0.02f> aux_automute;
	LpfIir<1.f / 4800.f> dcblock;

public:
	LoopingDelay(Params &params, DelayBuffer &delay_buffer)
		: params{params}
		, buf{delay_buffer} {
		Memory::clear();
		// for (auto &s : buf)
		// 	s = 0;
		//
	}

	// TODO: when global_mode[CALIBRATE] is set, we should change the audio callback
	void update(const AudioStreamConf::AudioInBlock &inblock, AudioStreamConf::AudioOutBlock &outblock) {
		// sz on the DLD is 8, but it's 64 here. sz/2 = AudioStreamConf::BlockSize
		constexpr uint32_t sz = AudioStreamConf::BlockSize * 2;
		constexpr uint32_t sz2 = AudioStreamConf::BlockSize;
		constexpr uint32_t blksz = sz2;

		Debug::Pin3::high();
		std::array<int32_t, AudioStreamConf::BlockSize> rd_buff; // on DLD this is 2x.. bug?
		std::array<int32_t, AudioStreamConf::BlockSize> rd_buff_dest;
		std::array<int32_t, AudioStreamConf::BlockSize> wr_buff;

		bool read_decrementing = doing_reverse_fade != params.modes.reverse;

		if (params.modes.inf == InfState::Off) {
			check_read_write_head_spacing();
			Memory::read(read_head, rd_buff.data(), blksz, 0, read_decrementing);
		} else {
			check_read_head_in_loop();
			bool did_cross_start_fade_addr =
				Memory::read(read_head, rd_buff.data(), blksz, calc_start_fade_addr(), read_decrementing);

			if (did_cross_start_fade_addr)
				start_looping_crossfade();
		}

		Memory::read(fade_dest_read_head, rd_buff_dest.data(), blksz, 0, false);

		for (auto [mem_wr, mem_rd, mem_rd_dest, out, in] : zip(wr_buff, rd_buff, rd_buff_dest, outblock, inblock)) {
			auto mainin = in.chan[0];
			auto auxin = in.chan[1];

			if (params.mute_on_boot_ctr) {
				params.mute_on_boot_ctr--;
				mainin = 0;
				auxin = 0;
			}

			if (params.settings.auto_mute) {
				mainin = main_automute.update(mainin);
				auxin = aux_automute.update(auxin);
			}

			// The Dry signal is just the clean signal, without any attenuation from LEVEL
			int32_t dry = mainin;

			// Read from the loop and save this value so we can output it to the Delay Out jack
			auto phase = (uint16_t)(4095.f * fade_read_phase);
			phase = __USAT(phase, 12);
			int32_t rd = ((float)mem_rd * epp_lut[phase]) + ((float)mem_rd_dest * epp_lut[4095 - phase]);

			if (params.settings.soft_clip)
				rd = compress(rd);

			if constexpr (Board::MemorySampleSize == 2)
				rd = __SSAT(rd, 16);

			// Attenuate the delayed signal with REGEN
			int32_t regen = (float)rd * params.feedback;

			// Attenuate the clean signal by the LEVEL parameter
			int32_t mainin_atten = (float)mainin * params.delay_feed;

			int32_t wr;
			int32_t auxout;
			if (params.modes.send_return_before_loop) {
				// Assign the auxin signal to the write head
				wr = auxin;
				// Add the loop contents to the input signal, and assign to the auxout signal
				auxout = (int32_t)(regen + mainin_atten);
			} else {
				// Add the loop contents to the input signal, as well as the auxin signal, and assign to the write head
				wr = (int32_t)(regen + mainin_atten + (float)auxin);
				// Assign the non-attenuated loop contents to the auxout signal
				auxout = rd;
			}

			// Wet/dry mix, as determined by the MIX parameter
			int32_t mix = clip(((float)dry * params.mix_dry) + ((float)rd * params.mix_wet));

			out.chan[0] = mix; // TODO: + CODEC_DAC_CALIBRATION_DCOFFSET
			out.chan[1] = auxout;

			// High-pass filter before writing to memory
			if (params.settings.runaway_dc_block)
				wr -= dcblock.update(wr);
			mem_wr = clip(wr);
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

	// Returns the address where we should start crossfading before looping
	uint32_t calc_start_fade_addr() {
		// For short periods (audio rate), disble crossfading before the end of the loop
		if (params.divmult_time < params.settings.crossfade_samples)
			return loop_end;
		else
			return Util::offset_samples(
				loop_end, params.settings.crossfade_samples / Board::MemorySampleSize, !params.modes.reverse);
		// FIXME: should that be / 2 (for 2 channels), not / sample size?
		//  Or else crossfade_samples should be named crossfade_bytes
	}

	void start_looping_crossfade() {
		constexpr uint32_t sz = AudioStreamConf::BlockSize * 2;
		params.reset_loopled_tmr();

		if (!is_crossfading()) {
			read_head = loop_start;
			fade_read_phase = 0.0;

			// Issue: is it necessary to set this below?
			fade_dest_read_head = Util::offset_samples(read_head, AudioStreamConf::BlockSize, !params.modes.reverse);
		} else {
			// Start fading from before the loop
			// We have to add in sz because read_addr has already been incremented by sz since a block was just
			// read
			uint32_t f_addr;
			if (params.modes.reverse)
				f_addr =
					Util::offset_samples(read_head, ((loop_start - loop_end) + sz) / Board::MemorySampleSize, false);
			else
				f_addr =
					Util::offset_samples(read_head, ((loop_end - loop_start) + sz) / Board::MemorySampleSize, true);

			// Issue: clearing a queued divmult time?
			start_crossfade(f_addr);
		}

		if (params.modes.inf == InfState::TransitioningOff)
			params.modes.inf = InfState::Off;
	}

	//// util:

	int32_t clip(int32_t val) {
		if (params.settings.soft_clip)
			val = compress(val);
		else if (Board::MemorySampleSize == 2)
			val = __SSAT(val, 16);
		return val;
	}

	uint32_t calculate_read_addr(uint32_t new_divmult_time) {
		return Util::offset_samples(write_head, new_divmult_time, !params.modes.reverse);
	}

	void set_divmult_time() {
		uint32_t use_ping_time = params.modes.ping_locked ? params.locked_ping_time : params.ping_time;
		uint32_t t_divmult_time = use_ping_time * params.time;
		// t_divmult_time = t_divmult_time & 0xFFFFFFFC; //force it to be a multiple of 4

		std::clamp(t_divmult_time, (uint32_t)0, Board::MemorySamplesNum);

		if (params.divmult_time == t_divmult_time)
			return;

		if (params.modes.inf == InfState::Off) {
			if (is_crossfading()) {
				fade_queued_divmult_time_end = t_divmult_time;
			} else {
				params.divmult_time = t_divmult_time;
				start_crossfade(calculate_read_addr(params.divmult_time));
			}
		} else {
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
		fade_dest_read_head = addr;
	}
};

} // namespace LDKit
