#pragma once
#include "conf/board_conf.hh"
#include "console.hh"
#include "controls.hh"
#include "hardware_tests/util.hh"
#include "libhwtests/AdcChecker.hh"
#include "libhwtests/AdcRangeChecker.hh"
#include "printf.h"
#include <string_view>

namespace LDKit::HWTests
{

struct TestADCs : IAdcChecker {
	static constexpr AdcRangeCheckerBounds bounds{
		.center_val = 2048,
		.center_width = 10,
		.center_check_counts = 10000,
		.min_val = Brain::PotAdcConf::min_value,
		.max_val = 4082,
	};

	uint32_t last_update = 0;

	Controls &controls;

	uint16_t window_min = 4095;
	uint16_t window_max = 0;

	static constexpr std::string_view pot_names[] = {"Time", "Feedback", "DelayFeed", "Mix"};
	static constexpr std::string_view bi_cv_names[] = {"Time"};
	static constexpr std::string_view uni_cv_names[] = {"Feedback", "DelayFeed", "Mix"};

	TestADCs(Controls &controls)
		: IAdcChecker{bounds, NumPots, 1, NumCVs - 1}
		, controls{controls} {}

	uint32_t get_adc_reading(uint8_t adc_i, AdcType adctype) override {
		uint16_t val = 0;

		if (adctype == AdcType::Pot)
			val = controls.read_pot(static_cast<PotAdcElement>(adc_i));

		if (adctype == AdcType::BipolarCV)
			val = controls.read_cv(TimeCV);

		if (adctype == AdcType::UnipolarCV)
			val = controls.read_cv(static_cast<CVAdcElement>(adc_i + 1));

		if (val > window_max)
			window_max = val;
		if (val < window_min)
			window_min = val;
		if ((HAL_GetTick() - last_update) > 250) {
			int32_t range = std::max(val - window_min, window_max - val);
			// window_ctr = 0;
			window_max = 0;
			window_min = 4095;
			printf_("\x1b[0GMin: %4d/%d  \tCur: %4d [range=%4d]\tMax: %4d/%d    ",
					get_last_min(),
					bounds.min_val,
					val,
					range,
					get_last_max(),
					bounds.max_val);
			last_update = HAL_GetTick();
		}
		return val;
	}

	void set_indicator(uint8_t adc_i, AdcType adctype, AdcCheckState state) override {
		static bool first_bipolar = true;
		static bool first_unipolar = true;

		if (state == AdcCheckState::NoCoverage) {
			if (adctype == AdcType::Pot) {
				printf_("\nChecking Pot %.10s (#%d):\n", pot_names[adc_i].data(), adc_i);
			}
			if (adctype == AdcType::BipolarCV) {
				printf_("\nChecking Bipolar CV Jack %.10s (#%d):\n", bi_cv_names[adc_i].data(), adc_i);
				if (first_bipolar)
					printf_("Patch Out L into Time CV (bi-polar CV)\n");
				first_bipolar = false;
			}
			if (adctype == AdcType::UnipolarCV) {
				printf_("\nChecking Unipolar CV Jack %.10s (#%d):\n", uni_cv_names[adc_i].data(), adc_i);
				if (first_unipolar)
					printf_("Patch Out R into the remaining CV jacks (uni-polar CV)\n");
				first_unipolar = false;
			}

			Board::LoopLED{}.set(false);
			Board::RevLED{}.set(true);
			Board::PingLED{}.set(true);
			Board::HoldLED{}.set(true);
		}

		if (state == AdcCheckState::AtMin) {
			Board::RevLED{}.set(false);
		}

		if (state == AdcCheckState::AtMax)
			Board::HoldLED{}.set(false);

		if (state == AdcCheckState::AtCenter)
			Board::PingLED{}.set(false);

		if (state == AdcCheckState::Elsewhere)
			Board::PingLED{}.set(true);

		if (state == AdcCheckState::FullyCovered) {
			printf_(" OK\n");
			Board::RevLED{}.set(false);
			Board::PingLED{}.set(false);
			Board::HoldLED{}.set(false);
		}
	}

	void pause_between_steps() override { HAL_Delay(10); }

	void show_multiple_nonzeros_error() override { Board::LoopLED{}.set(true); }

	bool button_to_skip_step() override {
		controls.ping_button.update();
		bool skip = controls.ping_button.is_just_pressed();
		if (skip)
			printf_(" XXX SKIPPED\n");
		return skip;
	}

	void delay_ms(uint32_t x) override { HAL_Delay(x); }
};
} // namespace LDKit::HWTests
