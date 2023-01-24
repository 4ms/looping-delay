#pragma once
#include "analog_in_ext.hh"
#include "conf/board_conf.hh"
#include "debug.hh"
#include "drivers/adc_builtin.hh"
#include "drivers/debounced_switch.hh"
#include "elements.hh"
#include "util/filter.hh"

namespace LDKit
{

class Controls {
	static constexpr bool debug = true;

	using enum mdrivlib::PinPolarity;
	template<typename ConfT>
	using AdcDmaPeriph = mdrivlib::AdcDmaPeriph<ConfT>;

	// ADCs (Pots and CV):
	std::array<uint16_t, NumCVs> cv_adc_buffer;
	AdcDmaPeriph<Board::CVAdcConf> cv_adcs{cv_adc_buffer, Board::CVAdcChans};

	std::array<uint16_t, NumPots> pot_adc_buffer;
	AdcDmaPeriph<Board::PotAdcConf> pot_adcs{pot_adc_buffer, Board::PotAdcChans};

	std::array<Oversampler<32, uint16_t>, NumPots> pots;
	std::array<Oversampler<8, uint16_t>, NumCVs> cvs;

public:
	// Buttons/Switches:
	Board::PingButton ping_button;
	Board::RevButton reverse_button;
	Board::HoldButton inf_button;
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

	uint16_t read_pot(PotAdcElement adcnum) {
		if constexpr (debug)
			return pot_adc_buffer[adcnum];
		else
			return pots[adcnum].val();
	}
	uint16_t read_cv(CVAdcElement adcnum) {
		if constexpr (debug)
			return cv_adc_buffer[adcnum];
		else
			return cvs[adcnum].val();
	}

	SwitchPos read_time_switch() { return static_cast<SwitchPos>(time_switch.read()); }

	void start() {
		if constexpr (!debug) {
			pot_adcs.dma.register_callback([this] {
				Debug::Pin1::high();
				for (unsigned i = 0; auto &pot : pots)
					pot.add_val(pot_adc_buffer[i++]);
				Debug::Pin1::low();
			});
			cv_adcs.dma.register_callback([this] {
				Debug::Pin0::high();
				for (unsigned i = 0; auto &cv : cvs)
					cv.add_val(cv_adc_buffer[i++]);
				Debug::Pin0::low();
			});
		}
		pot_adcs.start();
		cv_adcs.start();
	}

	void update() {
		ping_button.update();
		reverse_button.update();
		inf_button.update();

		ping_jack.update();
		reverse_jack.update();
		hold_jack.update();
	}

	void test() {
		ping_button.is_high();
		reverse_button.is_high();
		inf_button.is_high();

		ping_jack.is_high();
		reverse_jack.is_high();
		hold_jack.is_high();

		ping_led.high();
		// etc..
	}
};
} // namespace LDKit
