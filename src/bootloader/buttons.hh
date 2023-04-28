// TODO: don't include board_conf, make our own bootloader board_conf with just LEDS and buttons
#include "../conf/board_conf.hh"
#include "drivers/pin.hh"

namespace SamplerKit::Bootloader
{

static inline void init_buttons() {
	Board::PlayButton init_play_button;
	Board::RevButton init_rev_button;
	Board::BankButton init_bank_button;
}

enum class Button { Play, Rev, Bank };

static inline bool button_pushed(Button button) {
	if (button == Button::Play)
		return Board::PlayButton::PinT::read();
	else if (button == Button::Rev)
		return Board::RevButton::PinT::read();
	else
		return Board::BankButton::PinT::read();
}
} // namespace SamplerKit::Bootloader
