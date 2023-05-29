#pragma once
#include "brain_conf.hh"
#include "conf/flash_layout.hh"
#include "elements.hh"
#include "flash_block.hh"
#include "modes.hh"
#include "settings.hh"
#include "util/wear_level.hh"
#include <cstdint>

namespace LDKit
{

struct CalibrationData {
	uint32_t major_firmware_version;
	uint32_t minor_firmware_version;
	int32_t cv_calibration_offset[NumCVs];
	int32_t codec_adc_calibration_dcoffset[2];
	int32_t codec_dac_calibration_dcoffset[2];
	float tracking_comp;

	Settings settings;

	bool validate() {
		return (major_firmware_version < 100) && (minor_firmware_version < 100) &&					//
			   (major_firmware_version + minor_firmware_version > 0) &&								//
			   (std::abs(cv_calibration_offset[0]) < 200) &&										//
			   (std::abs(cv_calibration_offset[1]) < 200) &&										//
			   (std::abs(cv_calibration_offset[2]) < 200) &&										//
			   (std::abs(cv_calibration_offset[3]) < 200) &&										//
			   (tracking_comp > 0.5f) && (tracking_comp < 1.5f) &&									//
			   (uint8_t)settings.ping_method < (uint8_t)PingMethod::NUM_PING_METHODS &&				//
			   (uint8_t)settings.rev_jack <= 1 &&													//
			   (uint8_t)settings.inf_jack <= 1 &&													//
			   (uint8_t)settings.loop_clock <= 1 &&													//
			   (uint8_t)settings.main_clock <= 1 &&													//
			   settings.crossfade_samples > 48 && settings.crossfade_samples < 48000 &&				//
			   settings.write_crossfade_samples > 48 && settings.write_crossfade_samples < 48000 && //
			   std::abs(settings.crossfade_rate - settings.calc_fade_increment(settings.crossfade_samples)) < 0.01f &&
			   std::abs(settings.write_crossfade_rate -
						settings.calc_fade_increment(settings.write_crossfade_samples)) < 0.01f; // &&
	}
};

struct CalibrationStorage {
	WearLevel<FlashBlock<SettingsFlashAddr, CalibrationData, 8>> flash;
	CalibrationData cal_data;
	bool storage_is_ok = true;

	CalibrationStorage();
	bool save_flash_params();
	void factory_reset();

private:
	void apply_firmware_specific_adjustments();
	void handle_updated_firmware();
	void set_default_cal();
};

} // namespace LDKit
