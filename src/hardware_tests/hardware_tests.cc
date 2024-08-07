#include "audio_stream.hh"
#include "brain_conf.hh"
#include "calibration_storage.hh"
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
#include "switch.hh"
#include "system_target.hh"
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

void print_press_button() {
	printf_("%sPress button to continue%s\n", Term::BlinkGreen, Term::Normal);
}
void print_error(std::string_view err) {
	printf_("%s%.255s%s\n", Term::BoldRed, err.data(), Term::Normal);
}

void run(Controls &controls) {
	Board::PingJack ping_jack;
	Board::LoopClkBuilt loop_out;
	Board::LoopClkKit loop_passive;
	Board::ClkOut clk_out;
	Board::BusClkOut bus_clk_out;
	Board::LoopLED loop_led;

	printf_("\n\n%sLooping Delay Kit Hardware Test%s\n", Term::BoldGreen, Term::Normal);

	//////////////////////////////
	all_lights_off();
	Util::pause_until_button_released();

	// Display firmware version
	printf_("Firmware version %d.%d\n", FirmwareMajorVersion, FirmwareMinorVersion);
	for (unsigned i = 0; i < FirmwareMajorVersion; i++) {
		loop_led.high();
		HAL_Delay(150);
		loop_led.low();
		HAL_Delay(150);
	}
	HAL_Delay(350);
	for (unsigned i = 0; i < FirmwareMinorVersion; i++) {
		Board::HoldLED::set(true);
		HAL_Delay(150);
		Board::HoldLED::set(false);
		HAL_Delay(150);
	}

	print_press_button();
	printf_("Press Ping to continue, or hold Ping for five seconds to factory reset\n");
	Util::flash_mainbut_until_just_pressed();
	if (Util::check_for_longhold_button()) {
		printf_("Resetting...\n");
		PersistentStorage persistent_storage;
		persistent_storage.factory_reset();
		printf_("Sucess!\n");
		Util::flash_mainbut_until_pressed();
	}

	//////////////////////////////
	print_test_name("LED Test");
	printf_("Press the Play button to verify each LED\n");
	TestLEDs ledtester;
	ledtester.run_test();

	//////////////////////////////
	print_test_name("Button Test");
	printf_("Press each button once when it lights up\n");
	all_lights_off();
	TestButtons buttontester;
	buttontester.run_test();

	//////////////////////////////
	print_test_name("Switch Test");
	printf_("Flip the switch to all 3 positions\n");
	all_lights_off();
	TestSwitch switchtester;
	switchtester.run_test();

	//////////////////////////////
	print_test_name("Audio and Gate Output Test");
	SinOsc oscL{440.f, 2, 0, 48000};
	SinOsc oscR{2637.02f, 2, 0, 48000};
	AudioStream audio([&oscL, &oscR](const AudioStreamConf::AudioInBlock &in, AudioStreamConf::AudioOutBlock &out) {
		static uint32_t loopclk = 0;
		static uint32_t clkout = 0;
		for (auto [i, o] : zip(in, out)) {
			o.chan[0] = oscR.update() * 0x7FFFFF;
			o.chan[1] = oscL.update() * 0x7FFFFF;
		}
		Board::LoopClkBuilt::set((loopclk & 0b1));
		Board::LoopClkKit::set((loopclk & 0b1));
		loopclk++;
		Board::ClkOut::set((clkout & 0b11) == 0b00);
		clkout++;
	});
	audio.start();
	printf_("Verify:\n");
	printf_("  1) Audio Out: 440Hz sine, -10V to +10V [+/- 0.3V]\n");
	printf_("  2) Send: 2.7kHz sine, -10V to +10V [+/- 0.3V]\n");
	printf_("  3) Clk Out: 375Hz pulse/curvy-ramp wave (High 25%, low 75%), 0V to +8V [+/- 0.5V]\n");
	printf_("  4) Loop Clk Out: 750Hz square wave, 0V to +8V [+/- 0.5V]\n");

	print_press_button();
	Board::HoldLED{}.set(true);
	Util::flash_mainbut_until_pressed();
	Board::HoldLED{}.set(false);

	//////////////////////////////
	print_test_name("Audio Input Test");
	SinOsc osc3{377, 1.00f, 0.f, 48000};
	audio.set_callback([&osc3](const AudioStreamConf::AudioInBlock &in, AudioStreamConf::AudioOutBlock &out) {
		static uint32_t clkout = 0;
		for (auto [i, o] : zip(in, out)) {
			o.chan[0] = osc3.update() * 0x7FFFFF;
			o.chan[1] = i.chan[0] - i.chan[1];
		}
		Board::ClkOut::set((clkout & 0b11) == 0b00);
		clkout++;
	});
	printf_("  1) Patch Audio Out to scope, verify no signal\n");
	printf_("  2) Patch Send to Return, verify 377Hz sine 8Vpp wave [+/- 0.4V] on Out L\n");
	printf_("  3) Patch Clk Out to Audio In (keep other cable patched), verify beat freq 2Hz on Out L\n");

	print_press_button();
	Board::RevLED{}.set(true);
	Util::flash_mainbut_until_pressed();
	Board::RevLED{}.set(false);
	controls.ping_button.clear_events();

	//////////////////////////////
	print_test_name("Pot and CV jack Test");
	controls.start();

	printf_("Turn each pot from low to high to center\n");
	CenterFlatRamp test_waveform_0_5{1., 0.5, -4'800'000, 200'000, 0, 48000};
	CenterFlatRamp test_waveform_n5_5{1., 0.5, 4'800'000, -4'800'000, 0, 48000};
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
	printf_("Patch Clock Out into Hold Trig, then Ping Trig, then Rev Trig\n");
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

	HAL_Delay(200);
	SystemTarget::restart();

	while (true)
		;
}
} // namespace LDKit::HWTests
