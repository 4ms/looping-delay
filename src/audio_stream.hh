#pragma once

#include "audio_stream_conf.hh"
#include "conf/codec_conf.hh"
#include "conf/rcc_conf.hh"
#include "drivers/callable.hh"
#include "drivers/codec_WM8731.hh"
#include "drivers/i2c.hh"
#include <functional>

namespace LDKit
{

struct AudioStream {
	// TODO: Verify this region is non-cacheable:
	static inline AudioStreamConf::AudioInBuffer audio_in_dma_buffer;
	static inline AudioStreamConf::AudioOutBuffer audio_out_dma_buffer;

public:
	using AudioProcessFunction =
		std::function<void(const AudioStreamConf::AudioInBlock &, AudioStreamConf::AudioOutBlock &)>;

	AudioStream(AudioProcessFunction &&process_func)
		: codec_i2c{codec_i2c_conf}
		, codec{codec_i2c, sai_conf}
		, _process_func{std::move(process_func)} {

		codec.init();
		codec.set_rx_buffer_start(audio_in_dma_buffer[0]);
		codec.set_tx_buffer_start(audio_out_dma_buffer[0]);

		//{TC, HT}
		codec.set_callbacks([this] { _process<1>(); }, [this] { _process<0>(); });
	}

	void start() { codec.start(); }

	template<uint32_t buffer_half>
	void _process() {
		_process_func(audio_in_dma_buffer[buffer_half], audio_out_dma_buffer[buffer_half]);
	}

private:
	mdrivlib::I2CPeriph codec_i2c;
	mdrivlib::CodecWM8731 codec;

	AudioProcessFunction _process_func;
};

} // namespace LDKit
