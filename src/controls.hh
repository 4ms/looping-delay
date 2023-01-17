#pragma once
#include "analog_in_ext.hh"
#include "conf/board_conf.hh"
#include "drivers/adc_builtin.hh"
#include "drivers/debounced_switch.hh"
#include "elements.hh"
#include "util/filter.hh"

namespace LDKit
{

class Controls {
	using enum mdrivlib::PinPolarity;
	template<typename ConfT>
	using AdcDmaPeriph = mdrivlib::AdcDmaPeriph<ConfT>;

	// ADCs (Pots and CV):
	std::array<uint16_t, NumAdcs> adc_buffer;
	// AdcDmaPeriph<Board::AdcConf> adcs{adc_buffer, Board::AdcChans};

	AnalogIn<AdcDmaPeriph<Board::AdcConf>, NumAdcs, Oversampler<16, uint16_t>> adcs{adc_buffer, Board::AdcChans};

public:
	// Buttons/Switches:
	Board::PingButton ping_button;
	Board::RevButton reverse_button;
	Board::HoldButton hold_button;
	Board::TimeSwitch time_switch;

	// Trig Jacks
	Board::PingJack ping_jack;
	Board::RevJack reverse_jack;
	Board::HoldJack hold_jack;

	Board::ClkOut clk_out;
	Board::LoopClkOut loop_out;

	// LEDs:
	Board::PingLED ping_led;
	Board::HoldLED hold_led;
	Board::RevLED reverse_led;
	Board::ClkLED clk_led;

	enum class SwitchPos { Invalid = 0b00, Up = 0b01, Down = 0b10, Center = 0b11 };

	uint16_t read_adc(AdcElement adcnum) {
		return adcs.get(adcnum);
		// return adc_buffer[adcnum];
	}
	SwitchPos read_time_switch() { return static_cast<SwitchPos>(time_switch.read()); }

	void start() {
		adcs.start();
		//
	}

	void update() {
		ping_button.update();
		reverse_button.update();
		hold_button.update();

		ping_jack.update();
		reverse_jack.update();
		hold_jack.update();

		adcs.read();
	}

	void test() {
		ping_button.is_high();
		reverse_button.is_high();
		hold_button.is_high();

		ping_jack.is_high();
		reverse_jack.is_high();
		hold_jack.is_high();

		ping_led.high();
		// etc..
	}
};
} // namespace LDKit
