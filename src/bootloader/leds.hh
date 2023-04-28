#pragma once
#include "../conf/board_conf.hh"

namespace LDKit::Bootloader
{
enum class Leds {
	Hold,
	Ping,
	Rev,
};

class LEDs {
	LDKit::Board::HoldLED bank;
	LDKit::Board::PingLED play;
	LDKit::Board::RevLED rev;

public:
	LEDs() {
		set(Leds::Hold, false);
		set(Leds::Ping, false);
		set(Leds::Rev, false);
	}
	void set(Leds led, bool on) {
		switch (led) {
			case Leds::Hold:
				bank.set(on);
				break;
			case Leds::Ping:
				play.set(on);
				break;
			case Leds::Rev:
				rev.set(on);
				break;
		}
	}
};
} // namespace LDKit::Bootloader
