#include "drivers/pin.hh"

struct Debug {
	using Pin0 = mdrivlib::FPin<mdrivlib::GPIO::H, mdrivlib::PinNum::_13>;
	using Pin1 = mdrivlib::FPin<mdrivlib::GPIO::H, mdrivlib::PinNum::_14>;
	using Pin2 = mdrivlib::FPin<mdrivlib::GPIO::H, mdrivlib::PinNum::_15>;
	using Pin3 = mdrivlib::FPin<mdrivlib::GPIO::D, mdrivlib::PinNum::_3>;
};
