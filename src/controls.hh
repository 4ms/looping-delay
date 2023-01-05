#include "conf/control-conf.hh"
#include "drivers/adc_builtin.hh"

namespace LoopingDelay
{
struct Controls {

	mdrivlib::AdcDmaPeriph<BoardAdcConf> adcs;
};
} // namespace LoopingDelay
