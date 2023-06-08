// TODO: don't include board_conf, make our own bootloader board_conf with just LEDS and buttons
#include "../conf/board_conf.hh"
#include "drivers/pin.hh"

namespace LDKit::Bootloader
{

static inline void init_buttons() {
	Board::PingButton init_ping_button;
	Board::RevButton init_rev_button;
	Board::HoldButton init_hold_button;
}

enum class Button { Ping, Rev, Hold };

static inline bool button_pushed(Button button) {
	if (button == Button::Ping)
		return Board::PingButton::PinT::read();
	else if (button == Button::Rev)
		return Board::RevButton::PinT::read();
	else
		return Board::HoldButton::PinT::read();
}
} // namespace LDKit::Bootloader
