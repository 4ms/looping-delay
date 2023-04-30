#include "calibration_storage.hh"
#include "conf/flash_layout.hh"
#include "flash.hh"
#include <span>

namespace LDKit
{

void CalibrationStorage::set_default_cal() {
	cal_data.major_firmware_version = FirmwareMajorVersion;
	cal_data.minor_firmware_version = FirmwareMinorVersion;
	cal_data.cv_calibration_offset[0] = -Brain::CVAdcConf::bi_min_value;
	cal_data.cv_calibration_offset[1] = -Brain::CVAdcConf::uni_min_value;
	cal_data.cv_calibration_offset[2] = -Brain::CVAdcConf::uni_min_value;
	cal_data.cv_calibration_offset[3] = -Brain::CVAdcConf::uni_min_value;
	cal_data.tracking_comp = 1.f;
	cal_data.codec_adc_calibration_dcoffset[0] = 0;
	cal_data.codec_adc_calibration_dcoffset[1] = 0;
	cal_data.codec_dac_calibration_dcoffset[0] = 0;
	cal_data.codec_dac_calibration_dcoffset[1] = 0;
	cal_data.settings.auto_mute = true;
	cal_data.settings.soft_clip = true;
	cal_data.settings.dc_input = false;
	cal_data.settings.ping_method = PingMethod::IGNORE_FLAT_DEVIATION_10;
	cal_data.settings.rev_jack = GateType::Trig;
	cal_data.settings.inf_jack = GateType::Trig;
	cal_data.settings.loop_clock = GateType::Trig;
	cal_data.settings.main_clock = GateType::Gate;
	cal_data.settings.log_delay_feed = true;
	cal_data.settings.runaway_dc_block = true;
	cal_data.settings.auto_unquantize_timejack = true;
	cal_data.settings.send_return_before_loop = false;

	cal_data.settings.crossfade_samples = 4800;		  // SLOW_FADE_SAMPLES
	cal_data.settings.write_crossfade_samples = 4800; // FAST_FADE_SAMPLES

	cal_data.settings.crossfade_rate =
		cal_data.settings.calc_fade_increment(cal_data.settings.crossfade_samples); // SLOW_FADE_INCREMENT
	cal_data.settings.write_crossfade_rate =
		cal_data.settings.calc_fade_increment(cal_data.settings.crossfade_samples); // FAST_FADE_INCREMENT
}

void CalibrationStorage::factory_reset() {
	set_default_cal();
	save_flash_params();
}

CalibrationStorage::CalibrationStorage() {
	if (!flash.read(cal_data) || !cal_data.validate()) {
		set_default_cal();
		if (!flash.write(cal_data)) {
			storage_is_ok = false;
			// __BKPT(2); // ERROR!
		}
	}
	handle_updated_firmware();
}

bool CalibrationStorage::save_flash_params() { //
	if (!storage_is_ok)
		return false;
	return flash.write(cal_data);
}

void CalibrationStorage::handle_updated_firmware() {
	if (cal_data.major_firmware_version == FirmwareMajorVersion &&
		cal_data.minor_firmware_version == FirmwareMinorVersion)
		return;

	apply_firmware_specific_adjustments();
	cal_data.major_firmware_version = FirmwareMajorVersion;
	cal_data.minor_firmware_version = FirmwareMinorVersion;
	save_flash_params();
}

void CalibrationStorage::apply_firmware_specific_adjustments() {
	if (cal_data.major_firmware_version == 0 && cal_data.minor_firmware_version == 0) {
		// v0.0 => newer
	}
}

} // namespace LDKit
