#include "doctest.h"

#include "auto_mute.hh"
#include "wav_writer.hh"
#include <cstdio>

TEST_CASE("fade down") {
	constexpr uint32_t SampleRate = 48000;
	constexpr uint32_t DurationSec = 10;
	constexpr uint32_t NumSamples = SampleRate * DurationSec;

	// using Frame = std::pair<int32_t, int32_t>;
	// using Frame = std::tuple<int32_t, int32_t, int32_t>;
	struct Frame {
		int32_t first;
		int32_t second;
		int32_t third;
		int32_t fourth;
	};

	WavWriter<Frame> wav_writer{"test.wav", NumSamples, 4, SampleRate};

	constexpr double LPFCoef = 1. / 1000.;
	constexpr uint32_t LPFCoefRecip = 1 / LPFCoef;
	constexpr uint32_t MinLevel = 0x004000;
	constexpr uint32_t ADRate = 1000; // # samples to fade up or down

	AutoMuteClang<LPFCoefRecip, MinLevel, ADRate, ADRate> am;

	std::array<Frame, NumSamples> buf;

	constexpr double ToInt = 0x7FFF'FFF0;

	constexpr double base_freq = 4800.;
	constexpr double mod_freq = 4.;
	for (uint32_t i = 0; i < NumSamples; i++) {
		double base_sin = sin(2.0 * M_PI * i * base_freq / SampleRate);
		double mod_sin = std::max(0., sin(2.0 * M_PI * i * mod_freq / SampleRate));
		double sample = mod_sin * base_sin * 10;

		int32_t sample_i = sample * ToInt;
		auto out = am.update(sample_i);
		// double out_f = out / 8'388'608.0;

		buf[i].first = base_sin * ToInt;
		buf[i].second = out;
		buf[i].third = (double)am.get_env() * ToInt;
		buf[i].fourth = (double)am.get_lpf() * ToInt;

		// printf("%08x\t%f\t%f\n", sample_i, sample, out_f);
	}

	wav_writer.Write(buf);
}
