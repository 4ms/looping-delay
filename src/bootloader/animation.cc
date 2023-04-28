#include "bootloader/animation.hh"
#include "bootloader/leds.hh"
#include "drivers/stm32xx.h"
#include <cstdint>

namespace SamplerKit::Bootloader
{

namespace
{
LEDs leds;
}

void animate_signal(int32_t peak) {
	if (peak > 3000)
		leds.set(RgbLeds::Bank, Colors::red);
	else if (peak > 2500)
		leds.set(RgbLeds::Bank, Colors::yellow);
	else if (peak > 1000)
		leds.set(RgbLeds::Bank, Colors::green);
	else if (peak > 500)
		leds.set(RgbLeds::Bank, Colors::cyan);
	else if (peak > 200)
		leds.set(RgbLeds::Bank, Colors::blue);
	else
		leds.set(RgbLeds::Bank, Colors::off);
}

void animate(Animation animation_type) {
	uint32_t cur_tm = HAL_GetTick();
	static uint32_t last_tm = 0;
	static uint8_t ctr = 0;

	constexpr uint32_t TICKS_PER_MS = 1;
	uint32_t step_time = 500 * TICKS_PER_MS;

	switch (animation_type) {
		case Animation::RESET:
			leds.set(RgbLeds::Play, Colors::black);
			leds.set(RgbLeds::Rev, Colors::black);

			last_tm = cur_tm;
			ctr = 0;
			break;

		case Animation::SUCCESS:
			if (ctr == 0) {
				leds.set(RgbLeds::Play, Colors::red);
				leds.set(RgbLeds::Rev, Colors::red);
			} else if (ctr == 1) {
				leds.set(RgbLeds::Play, Colors::yellow);
				leds.set(RgbLeds::Rev, Colors::yellow);
			} else if (ctr == 2) {
				leds.set(RgbLeds::Play, Colors::white);
				leds.set(RgbLeds::Rev, Colors::white);
			} else if (ctr == 3) {
				leds.set(RgbLeds::Play, Colors::cyan);
				leds.set(RgbLeds::Rev, Colors::cyan);
			} else if (ctr == 4) {
				leds.set(RgbLeds::Play, Colors::green);
				leds.set(RgbLeds::Rev, Colors::green);
			} else if (ctr == 5) {
				leds.set(RgbLeds::Play, Colors::blue);
				leds.set(RgbLeds::Rev, Colors::blue);
			} else
				ctr = 0;
			break;

		case Animation::WAITING:
			// Flash button green/off when waiting
			if (ctr == 0)
				leds.set(RgbLeds::Play, Colors::black);
			else if (ctr == 1)
				leds.set(RgbLeds::Play, Colors::green);
			else
				ctr = 0;
			break;

		case Animation::RECEIVING:
			step_time = 200 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(RgbLeds::Play, Colors::blue);
				leds.set(RgbLeds::Rev, Colors::white);
			} else if (ctr == 1) {
				leds.set(RgbLeds::Play, Colors::white);
				leds.set(RgbLeds::Rev, Colors::blue);
			} else
				ctr = 0;
			break;

		case Animation::SYNC:
			step_time = 100 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(RgbLeds::Play, Colors::black);
				leds.set(RgbLeds::Rev, Colors::black);
			} else if (ctr == 1) {
				leds.set(RgbLeds::Play, Colors::magenta);
				leds.set(RgbLeds::Rev, Colors::black);
			} else
				ctr = 0;
			break;

		case Animation::WRITING:
			step_time = 100 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(RgbLeds::Play, Colors::black);
				leds.set(RgbLeds::Rev, Colors::yellow);
			} else if (ctr == 1) {
				leds.set(RgbLeds::Play, Colors::yellow);
				leds.set(RgbLeds::Rev, Colors::black);
			} else
				ctr = 0;
			break;

		case Animation::FAIL_ERR:
			step_time = 100 * TICKS_PER_MS;
			if (ctr == 0) {
				leds.set(RgbLeds::Play, Colors::black);
				leds.set(RgbLeds::Rev, Colors::red);
			} else if (ctr == 1) {
				leds.set(RgbLeds::Play, Colors::red);
				leds.set(RgbLeds::Rev, Colors::black);
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

} // namespace SamplerKit::Bootloader
