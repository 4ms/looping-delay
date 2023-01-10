#include "audio_stream.hh"
#include "conf/board_conf.hh"
#include "controls.hh"
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
	DelayBuffer audio_buffer = get_delay_buffer();

	LoopingDelay looping_delay{params, audio_buffer};
	AudioStream audio{[&looping_delay](const auto in, auto out) { looping_delay.update(in, out); }};
	mdrivlib::Timekeeper control_update_task{Board::control_read_tim_conf, [&]() { params.update(); }};

	control_update_task.start();
	audio.start();

	while (true) {
	}
}
