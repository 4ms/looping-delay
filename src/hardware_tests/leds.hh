#pragma once
#include "hardware_tests/util.hh"
#include "libhwtests/LEDTester.hh"

namespace LDKit::HWTests
{

struct TestLEDs : ILEDTester {
	TestLEDs()
		: ILEDTester{12} {}

	void set_led(int led_id, bool turn_on) override {
		switch (led_id) {
			case 0:
				Board::BankR::set(turn_on);
				break;
			case 1:
				Board::BankG::set(turn_on);
				break;
			case 2:
				Board::BankB::set(turn_on);
				break;
			case 3:
				Board::PlayR::set(turn_on);
				break;
			case 4:
				Board::PlayG::set(turn_on);
				break;
			case 5:
				Board::PlayB::set(turn_on);
				break;
			case 6:
				Board::RevR::set(turn_on);
				break;
			case 7:
				Board::RevG::set(turn_on);
				break;
			case 8:
				Board::RevB::set(turn_on);
				break;
			case 9:
				Board::BankR::set(turn_on);
				Board::BankG::set(turn_on);
				Board::BankB::set(turn_on);
				break;
			case 10:
				Board::PlayR::set(turn_on);
				Board::PlayG::set(turn_on);
				Board::PlayB::set(turn_on);
				break;
			case 11:
				Board::RevR::set(turn_on);
				Board::RevG::set(turn_on);
				Board::RevB::set(turn_on);
				break;
		}
	}

	void pause_between_steps() override {
		Util::pause_until_button_pressed();
		Util::pause_until_button_released();
	}
};
} // namespace LDKit::HWTests
