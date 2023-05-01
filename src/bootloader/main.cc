#include "bootloader.hh"

void main() {

	HAL_Init();
	LDKit::Bootloader::System system_init;
	LDKit::Bootloader::AudioBootloader bootloader;

	mdrivlib::Timekeeper update_task{{
										 .TIMx = TIM7,
										 .period_ns = 1'000'000'000 / 1'000, // 1'000Hz = 1ms
										 .priority1 = 1,
										 .priority2 = 1,
									 },
									 [&] { bootloader.update_LEDs(); }};
	update_task.start();

	if (bootloader.check_enter_bootloader())
		bootloader.run();

	mdrivlib::System::reset_buses();
	mdrivlib::System::reset_RCC();
	mdrivlib::System::jump_to(AppStartAddr);
	while (true) {
		__NOP();
	}
}

void recover_from_task_fault() {
	while (true) {
		__NOP();
	}
}
