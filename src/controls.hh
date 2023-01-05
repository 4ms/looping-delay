#include "conf/control-conf.hh"
#include "drivers/adc_builtin.hh"

namespace LoopingDelay
{

class Controls {
	// ADCs (Pots and CV):
	mdrivlib::AdcDmaPeriph<BoardAdcConf> adcs{adc_buffer, AdcConfs};
	std::array<uint16_t, NumAdcs> adc_buffer;

	// Buttons:

	// LEDs:

	//
public:
	void start() {
		adcs.start();
		//
	}

	uint16_t read_adc(AdcElement adcnum) { return adc_buffer[adcnum]; }
};
} // namespace LoopingDelay
