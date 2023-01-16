#include "audio_stream.hh"
#include "conf/board_conf.hh"
#include "controls.hh"
#include "debug.hh"
#include "delay_buffer.hh"
#include "drivers/timekeeper.hh"
#include "looping_delay.hh"
#include "system.hh"

namespace
{
LDKit::System _init;
}

void main() {
	using namespace LDKit;

	Controls controls;
	Params params{controls};
	DelayBuffer &audio_buffer = get_delay_buffer();
	LoopingDelay looping_delay{params, audio_buffer};
	AudioStream audio{[&looping_delay](const auto in, auto out) { looping_delay.update(in, out); }};

	// TODO: Make Params thread-safe:
	// Use double-buffering (two Params structs), and LoopingDelay is constructed with a Params*
	// And right before looping_delay.update(), call params.load_updated_values()
	//

	Debug::Pin2{};
	Debug::Pin3{};

	mdrivlib::Timekeeper params_update_task{Board::control_read_tim_conf, [&]() {
												Debug::Pin2::high();
												params.update();
												Debug::Pin2::low();
											}};

	params_update_task.start();
	audio.start();

	while (true) {
	}
}

void recover_from_task_fault() { NVIC_SystemReset(); }
