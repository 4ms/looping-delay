#pragma once
#include "calibration_storage.hh"
#include "controls.hh"
#include "flags.hh"
#include "modes.hh"
#include "settings.hh"

namespace LDKit
{

struct SystemMode {
	Controls &controls;
	Flags &flags;
	PersistentStorage &storage;
	Settings &settings;

private:
	bool ignore_inf_release = false;
	bool ignore_rev_release = false;
	bool ignore_ping_release = false;
	enum class Status { Normal, ExitArmed, Saving, Exit } status;
	enum class Page { EnterExit, AudioModes, PingDejitter } cur_page;
	uint32_t blink_tmr = 0;
	uint32_t pulse_ctr;
	uint32_t flash_ctr;
	bool ping_led_state = false;

public:
	SystemMode(Controls &controls, Flags &flags, PersistentStorage &storage)
		: controls{controls}
		, flags{flags}
		, storage{storage}
		, settings{storage.data.settings} {
	}

	void reset() {
		ignore_inf_release = true;
		ignore_rev_release = true;
		status = Status::Normal;
		cur_page = Page::AudioModes;
	}

	bool is_done() {
		return status == Status::Exit;
	}

	void update() {
		check_exit();

		update_page();

		display_settings();

		change_settings();

		if (controls.rev_button.is_just_released()) {
			ignore_rev_release = false;
		}
		if (controls.inf_button.is_just_released()) {
			ignore_inf_release = false;
		}
		if (controls.ping_button.is_just_released()) {
			ignore_ping_release = false;
		}
	}

private:
	void check_exit() {
		// Exit System Mode

		// If all three buttons are held for > 100ms, then we will exit
		if (!ignore_inf_release && controls.inf_button.how_long_held_pressed() > 100) {
			if (!ignore_rev_release && controls.rev_button.how_long_held_pressed() > 100) {
				if (!ignore_ping_release && controls.ping_button.how_long_held_pressed() > 100) {
					status = Status::ExitArmed;
				}
			}
		}

		// If all three buttons are held > 3s and TimeSwitch = Up, then save
		if (!ignore_inf_release && controls.inf_button.how_long_held_pressed() > 3000) {
			if (!ignore_rev_release && controls.rev_button.how_long_held_pressed() > 3000) {
				if (!ignore_ping_release && controls.ping_button.how_long_held_pressed() > 3000) {
					if (controls.read_time_switch() == Controls::SwitchPos::Center) {
						status = Status::Saving;
						storage.save_flash_params();
						ignore_inf_release = true;
						ignore_rev_release = true;
						ignore_ping_release = true;
					}
				}
			}
		}

		if (!controls.inf_button.is_pressed() && !controls.rev_button.is_pressed() &&
			!controls.ping_button.is_pressed())
		{
			if (status == Status::ExitArmed || status == Status::Saving)
				status = Status::Exit;
		}
	}

	void update_page() {
		if (controls.read_time_switch() == Controls::SwitchPos::Up)
			cur_page = Page::EnterExit;
		if (controls.read_time_switch() == Controls::SwitchPos::Center)
			cur_page = Page::AudioModes;
		if (controls.read_time_switch() == Controls::SwitchPos::Down) {
			if (cur_page != Page::PingDejitter)
				reset_ping_method_flash();
			cur_page = Page::PingDejitter;
		}
	}

	void change_settings() {
		switch (cur_page) {
			case Page::EnterExit: {
				break;
			}

			case Page::AudioModes: {
				if (controls.inf_button.is_just_released()) {
					if (!ignore_inf_release)
						settings.auto_mute = !settings.auto_mute;
					ignore_inf_release = false;
				}
				if (controls.ping_button.is_just_released()) {
					if (!ignore_ping_release)
						settings.soft_clip = !settings.soft_clip;
					ignore_ping_release = false;
				}
				if (controls.rev_button.is_just_released()) {
					if (!ignore_rev_release)
						settings.runaway_dc_block = !settings.runaway_dc_block;
					ignore_rev_release = false;
				}
				break;
			}

			case Page::PingDejitter: {
				if (controls.ping_button.is_pressed()) {
					constexpr uint32_t NumMethod = 5;
					auto old_ping_method = settings.ping_method;

					if (controls.read_pot(TimePot) < (4095 * 1 / NumMethod)) // 1 - 2
						settings.ping_method = PingMethod::IGNORE_FLAT_DEVIATION_10;

					else if (controls.read_pot(TimePot) < (4095 * 2 / NumMethod)) // 3 - 6
						settings.ping_method = PingMethod::IGNORE_PERCENT_DEVIATION;

					else if (controls.read_pot(TimePot) < (4095 * 3 / NumMethod)) // 7 - 10
						settings.ping_method = PingMethod::ONE_TO_ONE;

					else if (controls.read_pot(TimePot) < (4095 * 4 / NumMethod)) // 11 - 13
						settings.ping_method = PingMethod::MOVING_AVERAGE_2;

					else // 14 - 16
						settings.ping_method = PingMethod::LINEAR_AVERAGE_4;

					if (old_ping_method != settings.ping_method)
						reset_ping_method_flash();
				}
				if (controls.inf_button.is_just_released()) {
					if (!ignore_inf_release)
						settings.inf_jack = settings.inf_jack == GateType::Trig ? GateType::Gate : GateType::Trig;
					ignore_inf_release = false;
				}
				if (controls.rev_button.is_just_released()) {
					if (!ignore_rev_release)
						settings.rev_jack = settings.rev_jack == GateType::Trig ? GateType::Gate : GateType::Trig;
					ignore_rev_release = false;
				}
				break;
			}
		}
	}

	void display_settings() {
		if (status == Status::ExitArmed) {
			blink_tmr++;
			controls.inf_led.set((blink_tmr & 0x1FF) < 0x0FF);
			controls.ping_led.set((blink_tmr & 0x1FF) < 0x0FF);
			controls.reverse_led.set((blink_tmr & 0x1FF) < 0x0FF);
		}
		if (status == Status::Saving) {
			blink_tmr++;
			controls.inf_led.set((blink_tmr & 0xFF) < 0x7F);
			controls.ping_led.set((blink_tmr & 0xFF) < 0x7F);
			controls.reverse_led.set((blink_tmr & 0xFF) < 0x7F);
		}
		if (status == Status::Normal) {
			switch (cur_page) {
				case Page::AudioModes: {
					controls.inf_led.set(settings.auto_mute);
					controls.ping_led.set(settings.soft_clip);
					controls.reverse_led.set(settings.runaway_dc_block);
					break;
				}

				case Page::EnterExit: {
					// controls.ping_led.set(settings.main_clock == GateType::Gate);
					controls.inf_led.set(false);
					controls.ping_led.set(false);
					controls.reverse_led.set(false);
					break;
				}

				case Page::PingDejitter: {
					flash_ping_method();
					controls.inf_led.set(settings.inf_jack == GateType::Gate);
					controls.reverse_led.set(settings.rev_jack == GateType::Gate);
					break;
				}
			}
		}
	}

	void reset_ping_method_flash() {
		pulse_ctr = 0;
		flash_ctr = 0;
		ping_led_state = true;
		controls.ping_led.set(ping_led_state);
	}

	void flash_ping_method() {
		if (pulse_ctr)
			pulse_ctr--;
		else {
			ping_led_state = !ping_led_state;
			controls.ping_led.set(ping_led_state);

			if (flash_ctr) {
				pulse_ctr = 250;
				flash_ctr--;
			} else {
				pulse_ctr = 1000;
				flash_ctr = static_cast<std::underlying_type_t<PingMethod>>(settings.ping_method) * 2 + 1;
			}
		}
	}
};

} // namespace LDKit
