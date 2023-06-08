#include "audio_stream_conf.hh"
#include "drivers/codec_PCM3060_registers.hh"
#include "drivers/i2c_config_struct.hh"
#include "drivers/sai_config_struct.hh"

namespace LDKit::Board
{

using mdrivlib::GPIO;
using mdrivlib::I2CConfig;
using mdrivlib::PinAF;
using mdrivlib::PinNum;
using mdrivlib::SaiConfig;

// TODO: move to BrainBoard::
const I2CConfig codec_i2c_conf = {
	.I2Cx = I2C2,
	.SCL = {GPIO::D, PinNum::_7, PinAF::AltFunc4},
	.SDA = {GPIO::G, PinNum::_15, PinAF::AltFunc4},
	.timing =
		{
			.PRESC = 0x40,
			.SCLDEL_SDADEL = 0x50,
			.SCLH = 0x58,
			.SCLL = 0x74,
		},
	.priority1 = 0,
	.priority2 = 1,
};

// TODO: move to BrainBoard::
const SaiConfig sai_conf = {
	.sai = SAI2,
	.tx_block = SAI2_Block_A,
	.rx_block = SAI2_Block_B,

	.mode = SaiConfig::TXMaster,

	.dma_init_tx =
		{
			.DMAx = DMA2,
			.stream = DMA2_Stream1,
			.channel = DMA_REQUEST_SAI2_A,
			.IRQn = DMA2_Stream1_IRQn,
			.pri = 1,
			.subpri = 1,
		},
	.dma_init_rx =
		{
			.DMAx = DMA2,
			.stream = DMA2_Stream2,
			.channel = DMA_REQUEST_SAI2_B,
			.IRQn = DMA2_Stream2_IRQn,
			.pri = 1,
			.subpri = 1,
		},

	.datasize = AudioStreamConf::SampleBits == 24 ? SAI_DATASIZE_24 :
				AudioStreamConf::SampleBits == 16 ? SAI_DATASIZE_16 :
													SAI_DATASIZE_32,
	.framesize = 32,
	.samplerate = AudioStreamConf::SampleRate,

	.MCLK = {GPIO::E, PinNum::_0, PinAF::AltFunc10},
	.SCLK = {GPIO::D, PinNum::_13, PinAF::AltFunc10},
	.LRCLK = {GPIO::D, PinNum::_12, PinAF::AltFunc10},
	.SD_DAC = {GPIO::D, PinNum::_11, PinAF::AltFunc10}, // SD A
	.SD_ADC = {GPIO::G, PinNum::_10, PinAF::AltFunc10}, // SD B

	.reset_pin = {GPIO::E, PinNum::_12},

	.bus_address = 0,

	.num_tdm_ins = AudioStreamConf::NumInChans,
	.num_tdm_outs = AudioStreamConf::NumOutChans,

	.sync_send = SaiConfig::NoSendSync,
	.sync_receive_from = SaiConfig::NoReceiveSync,
};

} // namespace LDKit::Board
