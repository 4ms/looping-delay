#pragma once
#include "brain_conf.hh"
#include "conf/flash_layout.hh"
#include "elements.hh"
#include "flash_block.hh"
#include "util/wear_level.hh"
#include <cstdint>

namespace LDKit
{

struct CalibrationData {
	uint32_t major_firmware_version;
	uint32_t minor_firmware_version;
	int32_t cv_calibration_offset[NumCVs];
	int32_t unused0;
	int32_t codec_adc_calibration_dcoffset[2];
	int32_t codec_dac_calibration_dcoffset[2];
	uint32_t unused1;
	float tracking_comp;
	int32_t unused2;
	float unused3[3][3];
	// Note: Keep the unused elements here so flash layout is compatible with Sampler

	bool validate() {
		return (major_firmware_version < 100) && (minor_firmware_version < 100) &&
			   (major_firmware_version + minor_firmware_version > 0) && (tracking_comp > 0.5f) &&
			   (tracking_comp < 1.5f);
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
