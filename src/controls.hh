#include "conf/board_conf.hh"
#include "drivers/adc_builtin.hh"
#include "drivers/debounced_switch.hh"
#include "elements.hh"

namespace LoopingDelay
{

class Controls {
	using enum mdrivlib::PinPolarity;

	// ADCs (Pots and CV):
	mdrivlib::AdcDmaPeriph<Board::AdcConf> adcs{adc_buffer, Board::AdcChans};
	std::array<uint16_t, NumAdcs> adc_buffer;

	// Buttons/Switches:
	Board::PingButton ping_button;
	Board::RevButton reverse_button;
	Board::HoldButton hold_button;
	Board::TimeSwitch time_switch;

	// Trig Jacks
	Board::PingJack ping_jack;
	Board::RevJack reverse_jack;
	Board::HoldJack hold_jack;

	// LEDs:

	//
public:
	enum class SwitchPos { Invalid = 0b00, Up = 0b01, Down = 0b10, Center = 0b11 };

	void start() {
		adcs.start();
		//
	}

	uint16_t read_adc(AdcElement adcnum) { return adc_buffer[adcnum]; }
	SwitchPos read_time_switch() { return static_cast<SwitchPos>(time_switch.read()); }
	bool read_jack(TrigInJackElement jack) { return false; }
};
} // namespace LoopingDelay
