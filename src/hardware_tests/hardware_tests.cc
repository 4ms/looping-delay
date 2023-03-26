#include "audio_stream.hh"
#include "brain_conf.hh"
#include "conf/board_conf.hh"
#include "console.hh"
#include "drivers/ram_test.hh"
#include "drivers/stm32xx.h"
#include "hardware_tests/adc.hh"
#include "hardware_tests/buttons.hh"
#include "hardware_tests/gate_ins.hh"
#include "hardware_tests/leds.hh"
#include "hardware_tests/util.hh"
#include "libhwtests/CodecCallbacks.hh"
#include "printf.h"
#include "util/term_codes.hh"
#include "util/zip.hh"

namespace LDKit::HWTests
{

void all_lights_off() {
	Board::PingLED{}.set(false);
	Board::RevLED{}.set(false);
	Board::HoldLED{}.set(false);
	Board::LoopLED{}.set(false);
}

void print_test_name(std::string_view nm) {
	printf_("\n-------------------------------------\n");
	printf_("%s%.64s%s\n", Term::BoldYellow, nm.data(), Term::Normal);
}

void print_press_button() { printf_("%sPress button to continue%s\n", Term::BlinkGreen, Term::Normal); }
void print_error(std::string_view err) { printf_("%s%.255s%s\n", Term::BoldRed, err.data(), Term::Normal); }

void run(Controls &controls) {
	printf_("\n\n%sLooping Delay Kit Hardware Test%s\n", Term::BoldGreen, Term::Normal);

	//////////////////////////////
	all_lights_off();
	Util::pause_until_button_released();

	print_press_button();
	Util::flash_mainbut_until_pressed();

	//////////////////////////////
	print_test_name("LED Test");
	printf_("Press the Play button to verify each LED. You'll see red=>green=>blue\n");
	printf_("The LEDs will each turn white for you to verify color balance\n");
	TestLEDs ledtester;
	ledtester.run_test();

	//////////////////////////////
	print_test_name("Button Test");
	printf_("Press each button once when it lights up\n");
	all_lights_off();
	TestButtons buttontester;
	buttontester.run_test();

	//////////////////////////////
	print_test_name("Audio and Gate Output Test");
	SkewedTriOsc oscL{500, 0.3, 1, -1, 0, 48000};
	SkewedTriOsc oscR{3700, 0.85, 1, -1, 0, 48000};
	AudioStream audio([&oscL, &oscR](const AudioStreamConf::AudioInBlock &in, AudioStreamConf::AudioOutBlock &out) {
		static bool loopclk;
		static uint32_t clkout = 0;
		for (auto &o : out) {
			o.chan[0] = oscR.update() * 0x7FFFFF;
			o.chan[1] = oscL.update() * 0x7FFFFF;
		}
		Board::LoopClkOut::set(loopclk);
		Board::ClkOut::set((clkout & 0b11) == 0b00);
		clkout++;
	});
	audio.start();
	printf_("Verify:\n");
	printf_("  1) Out Left: 500Hz right-leaning triangle, -10V to +10V [+/- 0.3V]\n");
	printf_("  2) Out Right: 3700Hz left-leaning triangle, -10V to +10V [+/- 0.3V]\n");
	printf_("  3) Loop Clk Out: 750Hz square wave, 0V to +8V [+/- 0.5V]\n");
	printf_("  4) Clk Out: 375Hz square wave, 0V to +8V [+/- 0.5V]\n");

	print_press_button();
	Util::flash_mainbut_until_pressed();

	//////////////////////////////
	print_test_name("Audio Input Test");
	audio.set_callback([&oscR](const AudioStreamConf::AudioInBlock &in, AudioStreamConf::AudioOutBlock &out) {
		for (auto [i, o] : zip(in, out)) {
			o.chan[0] = oscR.update() * 0x7FFFFF;
			o.chan[1] = i.chan[0] - i.chan[1];
		}
	});
	printf_("  1) Patch Out L to scope, verify no signal\n");
	printf_("  2) Patch Out R to In R, verify left-leaning 3.4kHz 20Vpp wave [+/- 0.3V] on Out L\n");
	printf_("  3) Unpatch In R. Patch Out R to In L, verify right-leaning 3.4kHz 20Vpp wave [+/- 0.3V] on Out L\n");

	print_press_button();
	Util::flash_mainbut_until_pressed();
	controls.ping_button.clear_events();

	//////////////////////////////
	print_test_name("Pot and CV jack Test");
	controls.start();

	printf_("Turn each pot from low to high to center\n");
	CenterFlatRamp test_waveform_0_5{1., 0.3, -4'200'000, 300'000, 0, 48000};
	CenterFlatRamp test_waveform_n5_5{1., 0.3, 4'000'000, -4'000'000, 0, 48000};
	audio.set_callback([&](const AudioStreamConf::AudioInBlock &in, AudioStreamConf::AudioOutBlock &out) {
		for (auto &o : out) {
			o.chan[0] = test_waveform_0_5.update();	 // R
			o.chan[1] = test_waveform_n5_5.update(); // L
		}
	});

	TestADCs adctester{controls};
	adctester.run_test();

	all_lights_off();
	controls.ping_button.clear_events();

	//////////////////////////////
	print_test_name("Gate Input Test");
	printf_("Patch End Out into Play Trig, then Rev Trig\n");
	TestGateIns gateintester{controls};
	gateintester.run_test();

	all_lights_off();

	//////////////////////////////
	print_test_name("RAM Test (automatic)");
	printf_("If this takes longer than 20 seconds then RAM Test fails.\n");
	Board::HoldLED{}.set(true);
	auto err = mdrivlib::RamTest::test(Brain::MemoryStartAddr, Brain::MemorySizeBytes);
	if (err) {
		print_error("RAM Test Failed: readback did not match\n");
		while (1) {
			Board::PingLED{}.set(true);
			Board::RevLED{}.set(true);
			Board::HoldLED{}.set(true);
			Board::LoopLED{}.set(true);
			HAL_Delay(200);
			all_lights_off();
			HAL_Delay(200);
		}
	}
	Board::RevLED{}.set(true);
	Board::HoldLED{}.set(false);

	//////////////////////////////
	printf_("Hardware Test Complete.\n");

	printf_("Please reboot\n"); //, or system will automatically reboot in 5 seconds\n.");
	all_lights_off();

	while (true)
		;
}
} // namespace LDKit::HWTests
