#pragma once
#include "conf/board_conf.hh"
#include "drivers/stm32xx.h"
#include "libhwtests/HardwareTestUtil.hh"

namespace LDKit::HWTests
{
struct UtilIF {
	static bool main_button_pressed() { return Board::PingButton::PinT::read(); }
	static void delay_ms(uint32_t ms) { HAL_Delay(ms); }
	static void set_main_button_led(bool turn_on) { Board::PingLED::set(turn_on); }
};

using Util = HardwareTestUtil<UtilIF>;
} // namespace LDKit::HWTests
