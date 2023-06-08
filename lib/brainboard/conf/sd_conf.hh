#pragma once
#include "brain_conf.hh"
#include "brain_pinout.hh"
#include "drivers/sdcard_conf.hh"

namespace Brain
{
struct SDCardConf : mdrivlib::DefaultSDCardConf {
	static constexpr uint32_t SDPeriphNum = Brain::SdmmcPeriphNum;
	static constexpr uint32_t speed_hz = Brain::SdmmcMaxSpeed;
	static constexpr Width width = Brain::SdmmcWide ? Wide4 : Single;
	static constexpr mdrivlib::PinDef D0 = Brain::Pin::D16SdmmcDat0AF;
	static constexpr mdrivlib::PinDef D1 = Brain::Pin::D17SdmmcDat1AF;
	static constexpr mdrivlib::PinDef D2 = Brain::Pin::D18SdmmcDat2AF;
	static constexpr mdrivlib::PinDef D3 = Brain::Pin::D19SdmmcDat3AF;
	static constexpr mdrivlib::PinDef SCLK = Brain::Pin::D9SdmmcClkAF;
	static constexpr mdrivlib::PinDef CMD = Brain::Pin::D4SdmmcCmdAF;
};
} // namespace Brain
