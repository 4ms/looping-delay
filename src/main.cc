#include "audio_stream.hh"
#include "conf/board_conf.hh"
#include "controls.hh"
#include "debug.hh"
#include "delay_buffer.hh"
#include "drivers/timekeeper.hh"
#include "looping_delay.hh"
#include "system.hh"
#include "timer.hh"

// Testing
#include "util/oscs.hh"
TriangleOscillator<48000> osc1{1000};
TriangleOscillator<48000> osc2{15000};

namespace
{
LDKit::System _init;
}

void main() {
	using namespace LDKit;
	using AudioInBlock = AudioStreamConf::AudioInBlock;
	using AudioOutBlock = AudioStreamConf::AudioOutBlock;

	Debug::Pin0{};
	Debug::Pin1{};
	Debug::Pin2{};
	Debug::Pin3{};

	Controls controls;
	Flags flags;
	Timer timer;
	Params params{controls, flags, timer};
	DelayBuffer &audio_buffer = get_delay_buffer();
	LoopingDelay looping_delay{params, flags, audio_buffer};
	AudioStream audio([&looping_delay, &params](const AudioInBlock &in, AudioOutBlock &out) {
		// if (params.delay_feed > 0.5f) {
		// 	for (auto [o, i] : zip(out, in)) {
		// 		o.chan[0] = i.chan[0];
		// 		o.chan[1] = i.chan[1];
		// 	}
		// } else {
		// 	osc1.set_frequency(params.time * 100.f);
		// 	osc2.set_frequency(params.time * 1000.f);
		// 	for (auto [o, i] : zip(out, in)) {
		// 		o.chan[0] = (osc1.process_float() - 0.5f) * 0x00FFFFFFUL;
		// 		o.chan[1] = (osc2.process_float() - 0.5f) * 0x00FFFFFFUL;
		// 	}
		// }
		looping_delay.update(in, out);
	});

	// TODO: Make Params thread-safe:
	// Use double-buffering (two Params structs), and LoopingDelay is constructed with a Params*
	// And right before looping_delay.update(), call params.load_updated_values()

	__HAL_DBGMCU_FREEZE_TIM6();
	mdrivlib::Timekeeper params_update_task(Board::param_update_task_conf, [&params]() { params.update(); });

	timer.start();
	controls.start();
	params_update_task.start();
	audio.start();

	while (true) {
		__NOP();
	}
}

void recover_from_task_fault() { NVIC_SystemReset(); }
