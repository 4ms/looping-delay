#pragma once

#include "audio_stream_conf.hh"
#include "conf/codec_conf.hh"
#include "conf/rcc_conf.hh"
#include "drivers/callable.hh"
#include "drivers/codec_PCM3060.hh"
#include "drivers/i2c.hh"
#include <functional>

namespace LDKit
{

struct AudioStream {
	// TODO: Verify this region is non-cacheable:
	alignas(256) static inline AudioStreamConf::AudioInBuffer audio_in_dma_buffer;
	alignas(256) static inline AudioStreamConf::AudioOutBuffer audio_out_dma_buffer;

public:
	using AudioProcessFunction =
		std::function<void(const AudioStreamConf::AudioInBlock &, AudioStreamConf::AudioOutBlock &)>;

	AudioStream(AudioProcessFunction &&process_func)
		: codec_i2c{codec_i2c_conf}
		, codec{codec_i2c, sai_conf}
		, _process_func{std::move(process_func)} {

		codec.init();
		codec.set_rx_buffer_start(audio_in_dma_buffer);
		codec.set_tx_buffer_start(audio_out_dma_buffer);

		//{TC, HT}
		codec.set_callbacks([this] { _process<1>(); }, [this] { _process<0>(); });
	}

	void start() { codec.start(); }

	template<uint32_t buffer_half>
	void _process() {
		// NOTE: this only works when:
		// 	TC processes in[0] -> out[1] (dma just finished sending out[1], seems like it just finished filling in[0]?)
		// 	HT processes in[1] -> out[0] (dma just finished sending out[0], seems like it just finished filling in[1]?)
		_process_func(audio_in_dma_buffer[1 - buffer_half], audio_out_dma_buffer[buffer_half]);
	}

private:
	mdrivlib::I2CPeriph codec_i2c;
	mdrivlib::CodecPCM3060 codec;

	AudioProcessFunction _process_func;
};

} // namespace LDKit
