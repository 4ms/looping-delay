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
#include "flags.hh"
#include "loop_util.hh"
#include "params.hh"
#include "util/math.hh"
#include "util/zip.hh"
#include <algorithm>

namespace LDKit
{

class LoopingDelay {
	Params &params;
	Flags &flags;
	DelayBuffer &buf;

	uint32_t read_head; // read_addr
	uint32_t write_head;

	float read_fade_phase = 0;			   // read_fade_pos
	uint32_t read_fade_ending_addr;		   // fade_dest_read_addr
	uint32_t queued_divmult_time;		   // fade_queued_dest_divmult_time
	uint32_t queued_read_fade_ending_addr; // fade_queued_dest_read_addr

	float write_fade_phase = 0.f; // write_fade_pos
	enum class FadeState { NotFading, FadingDown, FadingUp, Crossfading };
	FadeState write_fade_state = FadeState::NotFading; // write_fade_state
	uint32_t write_fade_ending_addr;				   // fade_dest_write_addr

	uint32_t loop_start;
	uint32_t loop_end;

	bool doing_reverse_fade = false;

	AutoMute<0.0002f, 500, 0.02f, 0.02f> main_automute; // 1000 = audible tone
	AutoMute<0.0002f, 1000, 0.02f, 0.02f> aux_automute;
	DCBlock<1.f / 4800.f> dcblock;

public:
	LoopingDelay(Params &params, Flags &flags, DelayBuffer &delay_buffer)
		: params{params}
		, flags{flags}
		, buf{delay_buffer}
		, read_head{Brain::MemoryStartAddr}
		, write_head{Brain::MemoryStartAddr + 12000} {
		Memory::clear();
	}

	// TODO: when global_mode[CALIBRATE] is set, we should change the audio callback
	void update(const AudioStreamConf::AudioInBlock &inblock, AudioStreamConf::AudioOutBlock &outblock) {
		// sz on the DLD is 8, but it's 64 here. sz/2 = AudioStreamConf::BlockSize
		constexpr uint32_t sz = AudioStreamConf::BlockSize * 2;
		constexpr uint32_t blksz = AudioStreamConf::BlockSize;

		if (float amt = flags.take_scroll_amt(); amt != 0.f) {
			scroll_loop(amt);
		}

		// was process_mode_flags():
		if (flags.take_time_changed()) {
			// Debug::Pin1::high();
			set_divmult_time();
			// Debug::Pin1::low();
		}

		if (write_fade_state == FadeState::NotFading) {
			if (flags.take_inf_changed())
				toggle_inf();
		}

		if (!doing_reverse_fade) {
			if (flags.take_rev_changed())
				toggle_rev();
		}

		std::array<int16_t, AudioStreamConf::BlockSize> rd_buff; // on DLD this is 2x size.. bug?
		std::array<int16_t, AudioStreamConf::BlockSize> rd_buff_dest;
		std::array<int16_t, AudioStreamConf::BlockSize> wr_buff;

		// Read into rd_buff:
		bool read_decrementing = doing_reverse_fade != params.modes.reverse;
		if (params.modes.inf == InfState::Off) {
			check_read_write_head_spacing();
			Memory::read(read_head, rd_buff, 0, read_decrementing);
		} else {
			if (!check_read_head_in_loop()) {
				if (!is_crossfading()) {
					start_crossfade(loop_start);
					params.reset_loop();
				}
			}

			bool did_cross_start_fade_addr =
				Memory::read(read_head, rd_buff, calc_start_fade_addr(), read_decrementing);

			if (did_cross_start_fade_addr) {
				// Debug::Pin1::high();
				start_looping_crossfade();
				// Debug::Pin1::low();
			}
		}

		// Read into crossfading buffer (TODO: shouldn't this only happen if we're xfading?)
		Memory::read(read_fade_ending_addr, rd_buff_dest, 0, params.modes.reverse);

		for (auto [mem_wr, mem_rd, mem_rd_dest, out, in] : zip(wr_buff, rd_buff, rd_buff_dest, outblock, inblock)) {
			auto auxin = AudioStreamConf::AudioInFrame::sign_extend(in.chan[0]);
			auto mainin = AudioStreamConf::AudioInFrame::sign_extend(in.chan[1]);

			// Fix for noisy input on unmodded p3:
			// auxin = 0;
			//////////////////////////

			if (flags.mute_on_boot_ctr) {
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
			auto phase = (uint16_t)(4095.f * read_fade_phase);
			phase = __USAT(phase, 12);
			int32_t rd = ((float)mem_rd * epp_lut[phase]) + ((float)mem_rd_dest * epp_lut[4095 - phase]);
			rd *= 256;

			// rd = clip(rd);

			// Attenuate the delayed signal with REGEN
			int32_t regen = (float)rd * params.feedback;

			// Attenuate the clean signal by the LEVEL parameter
			int32_t mainin_atten = (float)mainin * params.delay_feed;

			int32_t wr;
			int32_t auxout;
			if (params.settings.send_return_before_loop) {
				wr = auxin;
				auxout = (int32_t)(regen + mainin_atten);
			} else {
				wr = (int32_t)(regen + mainin_atten + (float)auxin);
				auxout = rd;
			}

			// Wet/dry mix, as determined by the MIX parameter
			int32_t mix = ((float)dry * params.mix_dry) + ((float)rd * params.mix_wet);

			out.chan[0] = clip(auxout);
			out.chan[1] = clip(mix); // TODO: + CODEC_DAC_CALIBRATION_DCOFFSET

			// High-pass filter before writing to memory
			if (params.settings.runaway_dc_block)
				wr = dcblock.update(wr);
			mem_wr = clip(wr) / 256;
		}

		write_block_to_memory(wr_buff);

		increment_crossfading();
	}

	void write_block_to_memory(std::array<int16_t, AudioStreamConf::BlockSize> &wr_buff) {
		if (params.modes.inf == InfState::On)
			return;

		bool rev = params.modes.reverse;

		if (params.modes.inf == InfState::TransitioningOn) {
			if (write_fade_state == FadeState::FadingDown) {
				Memory::fade_write(write_fade_ending_addr, wr_buff, rev, 1.f - write_fade_phase);
				write_head = write_fade_ending_addr;
			}
		}

		if (params.modes.inf == InfState::TransitioningOff || params.modes.inf == InfState::Off) {
			if (write_fade_state == FadeState::Crossfading) {
				Memory::fade_write(write_fade_ending_addr, wr_buff, rev, write_fade_phase);
				// write in the opposite direction of [REV]
				Memory::fade_write(write_head, wr_buff, !rev, 1.f - write_fade_phase);
			} else if (write_fade_state == FadeState::FadingUp) {
				Memory::fade_write(write_fade_ending_addr, wr_buff, rev, write_fade_phase);
				write_head = write_fade_ending_addr;
			} else {
				Memory::write(write_head, wr_buff, rev);
				write_fade_ending_addr = write_head;
			}
		}
	}

	bool check_read_head_in_loop() {
		// If we're not crossfading, check if the read head is inside the loop
		return Util::in_between(read_head, loop_start, loop_end, params.modes.reverse);
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
				loop_end, params.settings.crossfade_samples /* / MemorySampleSize*/, !params.modes.reverse);
		// FIXME: should that be / 2 (for 2 channels), not / sample size?
		//  Or else crossfade_samples should be named crossfade_bytes
	}

	// When we near the end of the loop, start a crossfade to the beginning
	void start_looping_crossfade() {
		constexpr uint16_t sz = AudioStreamConf::BlockSize * 2;
		params.reset_loop();

		if (params.divmult_time < params.settings.crossfade_samples) {
			read_head = loop_start;
			read_fade_phase = 0.f;

			// Issue: is it necessary to set this below?
			read_fade_ending_addr = Util::offset_samples(read_head, AudioStreamConf::BlockSize, !params.modes.reverse);
		} else {
			// Start fading from before the loop
			// We have to add in sz because read_addr has already
			// been incremented by sz since a block was just read
			int32_t loop_size = loop_end - loop_start;
			if (params.modes.reverse)
				loop_size = -loop_size;

			uint32_t f_addr =
				Util::offset_samples(read_head, (loop_size + sz) / MemorySampleSize, !params.modes.reverse);

			// From DLD code : "Issue: clearing a queued divmult time"
			start_crossfade(f_addr);
		}

		if (params.modes.inf == InfState::TransitioningOff)
			params.set_inf_state(InfState::Off);
	}

	//// util:

	int32_t clip(int32_t val) {
		constexpr size_t Max24bit = (1U << 23);
		if (params.settings.soft_clip)
			val = compress<Max24bit, 0.75f>(val);
		else
			val = __SSAT(val, 24);
		return val;
	}

	uint32_t calculate_read_addr(uint32_t new_divmult_time) {
		return Util::offset_samples(write_head, new_divmult_time, !params.modes.reverse);
	}

	void set_divmult_time() {
		uint32_t use_ping_time = params.modes.ping_locked ? params.locked_ping_time : params.ping_time;
		uint32_t t_divmult_time = use_ping_time * params.time;
		// t_divmult_time = t_divmult_time & 0xFFFFFFFC; //force it to be a multiple of 4

		std::clamp(t_divmult_time, (uint32_t)0, MemorySamplesNum);

		if (params.divmult_time == t_divmult_time)
			return;

		if (params.modes.inf == InfState::Off) {
			if (is_crossfading()) {
				queued_divmult_time = t_divmult_time;
			} else {
				params.set_divmult(t_divmult_time);
				uint32_t t_read_addr = calculate_read_addr(params.divmult_time);
				if (t_read_addr != read_head)
					start_crossfade(t_read_addr);
			}
		} else {
			params.set_divmult(t_divmult_time);

			if (params.modes.adjust_loop_end)
				loop_end = Util::offset_samples(loop_start, t_divmult_time, params.modes.reverse);
			else
				loop_start = Util::offset_samples(loop_end, t_divmult_time, 1 - params.modes.reverse);

			// If the read addr is not in between the loop start and end, then fade to the loop start
			if (check_read_head_in_loop()) {
				if (is_crossfading()) {
					queued_read_fade_ending_addr = loop_start;
				} else {
					start_crossfade(loop_start);
					params.reset_loop();
				}
			}
		}
	}

	bool is_crossfading() { return read_fade_phase >= params.settings.crossfade_rate; }

	void start_crossfade(uint32_t addr) {
		read_fade_phase = params.settings.crossfade_rate;
		queued_divmult_time = 0; // means: no queued crossfade
		read_fade_ending_addr = addr;
	}

	void increment_crossfading() {
		if (read_fade_phase > 0.0f) {
			read_fade_phase += params.settings.crossfade_rate;
			if (read_fade_phase > 1.f) {
				read_fade_phase = 0.f;
				doing_reverse_fade = false;
				read_head = read_fade_ending_addr;

				if (queued_divmult_time > 0.f) {
					params.set_divmult(queued_divmult_time);
					start_crossfade(calculate_read_addr(queued_divmult_time));
				} else if (queued_read_fade_ending_addr) {
					start_crossfade(queued_read_fade_ending_addr);
					queued_read_fade_ending_addr = 0;
				}
			}
		}

		if (write_fade_phase > 0.f) {
			if (write_fade_state == FadeState::FadingUp)
				write_fade_phase += params.settings.write_crossfade_rate;

			else if (write_fade_state == FadeState::FadingDown)
				write_fade_phase += params.settings.crossfade_rate;

			else if (write_fade_state == FadeState::Crossfading)
				write_fade_phase += params.settings.write_crossfade_rate;

			if (write_fade_phase > 1.f) {
				write_fade_phase = 0.f;
				write_fade_state = FadeState::NotFading;
				write_head = write_fade_ending_addr;
				if (params.modes.inf == InfState::TransitioningOn)
					params.set_inf_state(InfState::On);
			}
		}
	}

	void toggle_rev() {
		params.toggle_reverse();
		if (params.modes.inf == InfState::Off)
			swap_read_write();
		else
			reverse_loop();

		doing_reverse_fade = true;
	}

	// When reversing in INF mode, swap the loop start/end
	// but offset them by the settings.crossfade_samples so the crossfade
	// stays within already recorded audio
	void reverse_loop() {
		uint32_t t = loop_start;
		uint32_t padding = params.settings.crossfade_samples;

		loop_start = Util::offset_samples(loop_end, padding, params.modes.reverse);
		loop_end = Util::offset_samples(t, padding, params.modes.reverse);

		// (Old TODO ToDo: ??? Add a crossfade for read head reversing direction here
		start_crossfade(read_head);
	}

	// When starting a reverse, crossfade a swap of the read/write heads
	void swap_read_write() {
		start_crossfade(write_fade_ending_addr);
		write_fade_ending_addr = read_head;
		write_fade_phase = params.settings.write_crossfade_rate;
		write_fade_state = FadeState::Crossfading;
	}

	void toggle_inf() {
		if (params.modes.inf == InfState::On || params.modes.inf == InfState::TransitioningOn) {
			params.set_inf_state(InfState::TransitioningOff);

			write_fade_phase = params.settings.write_crossfade_rate;
			write_fade_state = FadeState::FadingUp;
			write_fade_ending_addr = read_head;
		} else {
			// Don't change the loop start/end if we hit INF off recently
			// (recent enough that we're still TransitioningOff)
			// This is because the read and write heads are in the same spot
			if (params.modes.inf != InfState::TransitioningOff) {
				params.reset_loop();

				loop_start = read_fade_ending_addr;
				// set loop_start to the fade ending addr because if we happen to
				// be fading the read head when we hit inf (e.g. changing divmult time)
				// then we should loop between the new points since divmult_time
				// (used in the next line) corresponds with the fade ending addr

				loop_end = Util::offset_samples(loop_start, params.divmult_time, params.modes.reverse);
			}
			write_fade_phase = params.settings.crossfade_rate;
			write_fade_state = FadeState::FadingDown;
			write_fade_ending_addr = write_head;

			params.set_inf_state(InfState::TransitioningOn);
		}
	}

	void scroll_loop(float amt) {
		uint32_t hi = loop_end;
		uint32_t lo = loop_start;
		if (params.modes.reverse)
			std::swap(hi, lo);

		uint32_t loop_length = (hi > lo) ? (hi - lo) : hi + (Brain::MemorySizeBytes - lo);
		int32_t loop_shift = (int32_t)(amt * (float)loop_length);
		loop_shift /= MemorySampleSize;
		loop_start = Util::offset_samples(loop_start, loop_shift);
		loop_end = Util::offset_samples(loop_end, loop_shift);
	}
};

} // namespace LDKit
