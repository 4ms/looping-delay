#include "bootloader/animation.hh"
#include "bootloader/leds.hh"
#include "drivers/stm32xx.h"
#include <cstdint>

namespace LDKit::Bootloader
{

namespace
{
LEDs leds;
}

void animate_signal(int32_t peak) {
	uint32_t tmr = HAL_GetTick() & 255;

	if (peak > 3000)
		leds.set(Leds::Hold, true);
	else if (peak > 2500)
		leds.set(Leds::Hold, tmr > 200);
	else if (peak > 1000)
		leds.set(Leds::Hold, tmr > 127);
	else if (peak > 500)
		leds.set(Leds::Hold, tmr > 100);
	else if (peak > 200)
		leds.set(Leds::Hold, tmr > 50);
	else
		leds.set(Leds::Hold, false);
}

void animate(Animation animation_type) {
	uint32_t cur_tm = HAL_GetTick();
	static uint32_t last_tm = 0;
	static uint8_t ctr = 0;

	constexpr uint32_t TICKS_PER_MS = 1;
	uint32_t step_time = 500 * TICKS_PER_MS;

	switch (animation_type) {
		case Animation::RESET:
			leds.set(Leds::Hold, false);
			leds.set(Leds::Ping, false);
			leds.set(Leds::Rev, false);

			last_tm = cur_tm;
			ctr = 0;
			break;

		case Animation::SUCCESS:
			if (ctr == 0) {
				leds.set(Leds::Hold, true);
				leds.set(Leds::Ping, false);
				leds.set(Leds::Rev, false);
			} else if (ctr == 1) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, true);
				leds.set(Leds::Rev, false);
			} else if (ctr == 2) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, false);
				leds.set(Leds::Rev, true);
			} else if (ctr == 3) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, true);
				leds.set(Leds::Rev, false);
			} else
				ctr = 0;
			break;

		case Animation::WAITING:
			// Flash button when waiting
			if (ctr == 0) {
				leds.set(Leds::Hold, true);
				leds.set(Leds::Ping, true);
				leds.set(Leds::Rev, true);
			} else if (ctr == 1) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, false);
				leds.set(Leds::Rev, false);
			} else
				ctr = 0;
			break;

		case Animation::RECEIVING:
			step_time = 200 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, true);
				leds.set(Leds::Rev, false);
			} else if (ctr == 1) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, false);
				leds.set(Leds::Rev, true);
			} else
				ctr = 0;
			break;

		case Animation::SYNC:
			step_time = 100 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(Leds::Hold, true);
			} else if (ctr == 1) {
				leds.set(Leds::Hold, false);
			} else
				ctr = 0;
			break;

		case Animation::WRITING:
			step_time = 100 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(Leds::Hold, true);
				leds.set(Leds::Ping, false);
				leds.set(Leds::Rev, true);
			} else if (ctr == 1) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, true);
				leds.set(Leds::Rev, false);
			} else
				ctr = 0;
			break;

		case Animation::FAIL_ERR:
			step_time = 100 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, false);
				leds.set(Leds::Rev, true);
			} else if (ctr == 1) {
				leds.set(Leds::Hold, false);
				leds.set(Leds::Ping, false);
				leds.set(Leds::Rev, false);
			} else
				ctr = 0;
			break;

		default:
			break;
	}

	if ((cur_tm - last_tm) > step_time) {
		ctr++;
		last_tm = cur_tm;
	}
}

} // namespace LDKit::Bootloader
