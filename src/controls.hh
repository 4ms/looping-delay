#include "conf/control-conf.hh"
#include "drivers/adc_builtin.hh"

namespace LoopingDelay
{
struct Controls {

	mdrivlib::AdcDmaPeriph<BoardAdcConf> adcs{adc_buffer, AdcConfs};

	std::array<uint16_t, NumAdcs> adc_buffer;
};
} // namespace LoopingDelay
