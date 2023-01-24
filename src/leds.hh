#include "controls.hh"
#include "modes.hh"

namespace LDKit
{

struct Leds {
	Controls &controls;
	ChannelMode &modes;

	Leds(Controls &controls, ChannelMode &modes)
		: controls{controls}
		, modes{modes} {}

	void update() {
		if (modes.inf == InfState::TransitioningOn)
			controls.hold_led.high();
		else if (modes.inf == InfState::TransitioningOff)
			controls.hold_led.low();

		controls.reverse_led.set(modes.reverse);
	}
};

} // namespace LDKit
