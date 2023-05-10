#include "auto_mute.hh"
#include "wav_writer.hh"
#include <cstdio>

int main() {
	// Settings:
	// Note: mV values are relative to an assumed 20Vpp max input signal
	// No matter what, there is always this amount of signal
	constexpr double NoiseFloor_mV = 2.;

	// LPF coef for determining average amplitude of the signal
	constexpr double LPFCoef = 0.002;

	// Lowest level of envelope
	constexpr double Threshold_mV = 10.;

	// # of samples to fade up or down when crossing threshold
	constexpr uint32_t ADRate = 10000;

	constexpr uint32_t SampleRate = 48000;
	constexpr uint32_t DurationSec = 10;
	constexpr double base_freq = 1000.; // Hz
	constexpr double mod_freq = 0.2;	// Hz

	////////

	constexpr uint32_t NumSamples = SampleRate * DurationSec;
	constexpr double ToInt = 0x7FFF'0000UL;
	constexpr uint32_t LPFCoefRecip = 1 / LPFCoef;
	constexpr double Threshold_f = Threshold_mV / (20. * 1000.);
	constexpr uint32_t Threshold = Threshold_f * ToInt;
	constexpr double NoiseFloorRatio = NoiseFloor_mV / (20. * 1000.);

	struct Frame {
		int32_t input;
		int32_t output;
		int32_t envelope;
		int32_t input_amplitude;
	};

	AutoMute<LPFCoefRecip, Threshold, ADRate, ADRate> am;
	std::array<Frame, NumSamples> buf;
	WavWriter<Frame> wav_writer{"test.wav", NumSamples, 4, SampleRate};

	for (uint32_t i = 0; i < NumSamples; i++) {
		double base_sin = sin(2.0 * M_PI * i * base_freq / SampleRate);
		double mod_sin = 0.4 + 0.5 * sin(2.0 * M_PI * i * mod_freq / SampleRate);
		mod_sin = std::max(NoiseFloorRatio, mod_sin);
		double sample = mod_sin * base_sin;

		int32_t sample_i = sample * ToInt;
		auto out = am.update(sample_i);

		buf[i].input = sample_i;
		buf[i].output = out;
		buf[i].envelope = (double)am.fade_amt * ToInt;
		buf[i].input_amplitude = (double)am.lpf;
	}

	wav_writer.Write(buf);

	return 0;
}
