#pragma once
#include "../conf/board_conf.hh"

namespace SamplerKit::Bootloader
{
enum class RgbLeds {
	Bank,
	Play,
	Rev,
};

class LEDs {
	SamplerKit::Board::BankLED bank;
	SamplerKit::Board::PlayLED play;
	SamplerKit::Board::RevLED rev;

public:
	LEDs() {
		set(RgbLeds::Bank, Colors::off);
		set(RgbLeds::Play, Colors::off);
		set(RgbLeds::Rev, Colors::off);
	}
	void set(RgbLeds led, Color color) {
		switch (led) {
			case RgbLeds::Bank:
				bank.set_color(color);
				break;
			case RgbLeds::Play:
				play.set_color(color);
				break;
			case RgbLeds::Rev:
				rev.set_color(color);
				break;
		}
	}
};
} // namespace SamplerKit::Bootloader
