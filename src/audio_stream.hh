#pragma once

#include "audio_stream_conf.hh"
#include "conf/codec_conf.hh"
#include "conf/rcc_conf.hh"
#include "drivers/callable.hh"
#include "drivers/codec_WM8731.hh"
#include "drivers/i2c.hh"

namespace LoppingDelay
{

struct AudioStream {
	mdrivlib::I2CPeriph codec_i2c;
	mdrivlib::CodecWM8731 codec;

	// TODO: Verify this region is non-cacheable:
	static inline AudioStreamConf::AudioInBlock audio_in_dma_block;
	static inline AudioStreamConf::AudioOutBlock audio_out_dma_block;

public:
	using AudioProcessFunction =
		Function<void(const AudioStreamConf::AudioInBuffer &, AudioStreamConf::AudioOutBuffer &)>;

	AudioStream()
		: codec_i2c{codec_i2c_conf}
		, codec{codec_i2c, sai_conf} {

		// Setup clocks needed for codec
		// Must const_cast because STM32-HAL is not const-correct
		HAL_RCCEx_PeriphCLKConfig(const_cast<RCC_PeriphCLKInitTypeDef *>(&sai_rcc_clk_conf));

		codec.init();
		codec.set_rx_buffer_start(audio_in_dma_block[0]);
		codec.set_tx_buffer_start(audio_out_dma_block[0]);
	}

	void set_process_function(AudioProcessFunction &process) { _process_func = process; }

	void start(AudioProcessFunction &process) {
		_process_func = process;
		codec.set_callbacks([this] { _process<1>(); }, [this] { _process<0>(); });
		codec.start();
	}

	template<uint32_t buffer_half>
	void _process() {
		_process_func(audio_in_dma_block[buffer_half], audio_out_dma_block[buffer_half]);
	}

private:
	AudioProcessFunction _process_func;
	float audio_load_smoothed = 0.f;
	static constexpr float kSmoothCoef = 0.01; // 100 process blocks = 6400 samples: tau=0.133s
	static constexpr float kinvSmoothCoef = 1.f - kSmoothCoef;
};

} // namespace LoppingDelay
