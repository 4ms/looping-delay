#pragma once
#include "hardware_tests/util.hh"
#include "libhwtests/LEDTester.hh"

namespace LDKit::HWTests
{

struct TestLEDs : ILEDTester {
	TestLEDs()
		: ILEDTester{4} {}

	void set_led(int led_id, bool turn_on) override {
		switch (led_id) {
			case 0:
				Board::HoldLED::set(turn_on);
				break;
			case 1:
				Board::PingLED::set(turn_on);
				break;
			case 2:
				Board::RevLED::set(turn_on);
				break;
			case 3:
				Board::LoopLED::set(turn_on);
				break;
		}
	}

	void pause_between_steps() override {
		Util::pause_until_button_pressed();
		Util::pause_until_button_released();
	}
};
} // namespace LDKit::HWTests
