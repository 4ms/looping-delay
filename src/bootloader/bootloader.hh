#include "audio_stream.hh"
#include "bootloader/animation.hh"
#include "bootloader/buttons.hh"
#include "bootloader/leds.hh"
#include "conf/bootloader_settings.hh"
#include "conf/flash_layout.hh"
#include "drivers/stm32xx.h"
#include "flash.hh"
#include "system.hh"
#include "util/analyzed_signal.hh"
#include "util/zip.hh"
#include <cstring> //for memcpy: adds 100Bytes to binary vs. using our own

// #define USING_FSK
#define USING_QPSK

#ifdef USING_QPSK
#include "stm_audio_bootloader/qpsk/demodulator.h"
#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#else
#include "stm_audio_bootloader/fsk/demodulator.h"
#include "stm_audio_bootloader/fsk/packet_decoder.h"
#endif

namespace LDKit::Bootloader
{

struct AudioBootloader {
#ifdef USING_QPSK
	static constexpr float kModulationRate = 6000.0;
	static constexpr float kBitRate = 12000.0;
	static constexpr float kSampleRate = 48000.0;
#else
	static constexpr uint32_t kSampleRate = BootloaderConf::SampleRate;		 //-s
	static constexpr uint32_t kPausePeriod = BootloaderConf::Encoding.blank; //-b
	static constexpr uint32_t kOnePeriod = BootloaderConf::Encoding.one;	 //-n
	static constexpr uint32_t kZeroPeriod = BootloaderConf::Encoding.zero;	 //-z
#endif
	static constexpr uint32_t kStartReceiveAddress = BootloaderReceiveAddr;
	static constexpr uint32_t kBlkSize = BootloaderConf::ReceiveSectorSize;					   // Flash page size, -g
	static constexpr uint16_t kPacketsPerBlock = kBlkSize / stm_audio_bootloader::kPacketSize; // kPacketSize=256

	uint8_t recv_buffer[kBlkSize];

	stm_audio_bootloader::PacketDecoder decoder;
	stm_audio_bootloader::Demodulator demodulator;

	uint16_t packet_index = 0;
	uint16_t discard_samples = 16000;
	uint32_t current_flash_address;

	int32_t atten = 256;

	enum UiState { UI_STATE_WAITING, UI_STATE_RECEIVING, UI_STATE_ERROR, UI_STATE_WRITING, UI_STATE_DONE };
	UiState ui_state;

	PeakMeter<int32_t> meter;

	AudioBootloader() {
		init_buttons();
		animate(Animation::RESET);
	}

	bool check_enter_bootloader() {
		HAL_Delay(300);

		uint32_t dly = 32000;
		uint32_t button_debounce = 0;
		while (dly--) {
			if (button_pushed(Button::Rev) && button_pushed(Button::Bank))
				button_debounce++;
			else
				button_debounce = 0;
		}
		HAL_Delay(100);
		bool do_bootloader = (button_debounce > 15000);
		return do_bootloader;
	}

	// GCC_OPTIMIZE_OFF
	void run() {
		init_reception();

		AudioStream audio_stream(
			[this](const AudioStreamConf::AudioInBlock &inblock, AudioStreamConf::AudioOutBlock &outblock) {
				for (auto [in, out] : zip(inblock, outblock)) {
					if (!discard_samples) {
#ifdef USING_FSK
						bool sample = in.sign_extend_chan(1) > 1000;
						demodulator.PushSample(sample ? 1 : 0);
						out.chan[0] = in.chan[1];
						out.chan[1] = sample ? 0xC00000 : 0x400000;
#else
						int32_t sample = (in.sign_extend_chan(1) / atten);
						meter.update(sample);
						demodulator.PushSample(sample);
						out.chan[0] = in.chan[1];
						out.chan[1] = sample;
#endif
					} else {
						--discard_samples;
					}
				}
			});
		audio_stream.start();

		uint32_t button_exit_armed = 0;
		uint32_t rev_but_armed = 0;

		while (button_pushed(Button::Rev) || button_pushed(Button::Bank))
			;

		HAL_Delay(300);

		uint8_t exit_updater = false;
		while (!exit_updater) {
			bool rcv_err = false;

			if (demodulator.state() == stm_audio_bootloader::DEMODULATOR_STATE_OVERFLOW) {
				rcv_err = true;
			} else {
				demodulator.ProcessAtLeast(16);
			}
			while (demodulator.available() && !rcv_err && !exit_updater) {
				uint8_t symbol = demodulator.NextSymbol();
				auto state = decoder.ProcessSymbol(symbol);

				switch (state) {
					case stm_audio_bootloader::PACKET_DECODER_STATE_SYNCING:
						animate(Animation::SYNC);
						break;

					case stm_audio_bootloader::PACKET_DECODER_STATE_OK:
						ui_state = UI_STATE_RECEIVING;
						memcpy(recv_buffer + (packet_index % kPacketsPerBlock) * stm_audio_bootloader::kPacketSize,
							   decoder.packet_data(),
							   stm_audio_bootloader::kPacketSize);
						++packet_index;
						if ((packet_index % kPacketsPerBlock) == 0) {
							ui_state = UI_STATE_WRITING;
							bool write_ok = write_buffer();
							if (!write_ok) {
								ui_state = UI_STATE_ERROR;
								rcv_err = true;
							}
							new_block();
						} else {
							new_packet();
						}
						break;

					case stm_audio_bootloader::PACKET_DECODER_STATE_ERROR_SYNC:
						rcv_err = true;
						// Console::write("Sync Error\n");
						break;

					case stm_audio_bootloader::PACKET_DECODER_STATE_ERROR_CRC:
						rcv_err = true;
						// Console::write("CRC Error\n");
						break;

					case stm_audio_bootloader::PACKET_DECODER_STATE_END_OF_TRANSMISSION:
						// Console::write("End\n");
						// Write out buffer if we haven't yet (which happens if we load only to RAM)
						if (current_flash_address == kStartReceiveAddress) {
							if (!write_buffer()) {
								ui_state = UI_STATE_ERROR;
								rcv_err = true;
								// Console::write("Buffer Write Error\n");
								new_block();
								break;
							}
						}
						exit_updater = true;
						ui_state = UI_STATE_DONE;
						copy_firmware();
						// Console::write("Success!\n");
						animate_until_button_pushed(Animation::SUCCESS, Button::Play);
						animate(Animation::RESET);
						HAL_Delay(100);
						break;

					default:
						break;
				}
			}
			if (rcv_err) {
				ui_state = UI_STATE_ERROR;
				animate_until_button_pushed(Animation::FAIL_ERR, Button::Play);
				animate(Animation::RESET);
				HAL_Delay(100);
				init_reception();
				exit_updater = false;
			}

			if (button_pushed(Button::Rev)) {
				if (rev_but_armed) {
					HAL_Delay(100);
					init_reception();
				}
				rev_but_armed = 0;
			} else
				rev_but_armed = 1;

			if (button_pushed(Button::Play)) {
				if (button_exit_armed) {
					if (ui_state == UI_STATE_WAITING) {
						exit_updater = true;
					}
				}
				button_exit_armed = 0;
			} else
				button_exit_armed = 1;
		}
		ui_state = UI_STATE_DONE;
		while (button_pushed(Button::Play) || button_pushed(Button::Rev)) {
			;
		}
	}

	void init_reception() {
#ifdef USING_QPSK
		// QPSK
		decoder.Init((uint16_t)20000);
		demodulator.Init(
			kModulationRate / kSampleRate * 4294967296.0f, kSampleRate / kModulationRate, 2.f * kSampleRate / kBitRate);
		demodulator.SyncCarrier(true);
		decoder.Reset();
#else
		// FSK
		decoder.Init();
		decoder.Reset();
		demodulator.Init(kPausePeriod, kOnePeriod, kZeroPeriod); // pause_thresh = 24. one_thresh = 6.
		demodulator.Sync();
#endif
		current_flash_address = kStartReceiveAddress;
		packet_index = 0;
		ui_state = UI_STATE_WAITING;
	}

	bool write_buffer() {
		// Console::write("Writing\n");
		if ((current_flash_address + kBlkSize) <= get_sector_addr(NumFlashSectors)) {
			flash_write_page(recv_buffer, current_flash_address, kBlkSize);
			current_flash_address += kBlkSize;
			return true;
		} else {
			return false;
		}
	}

	void copy_firmware() {
		if (kStartReceiveAddress != AppFlashAddr) {
			// Console::write("Copying from receive sectors to execution sectors\n");
			uint32_t src_addr = kStartReceiveAddress;
			uint32_t dst_addr = AppFlashAddr;
			while (dst_addr < kStartReceiveAddress) {
				flash_write_page(reinterpret_cast<const uint8_t *>(src_addr), dst_addr, 16 * 1024);
				src_addr += 16 * 1024;
				dst_addr += 16 * 1024;
			}
		}
	}

	void update_LEDs() {
		int32_t max = meter.max;
		int32_t min = meter.min;
		meter.reset();
		auto peak = std::max(std::abs(max), std::abs(min));
		animate_signal(peak);

		if (ui_state == UI_STATE_RECEIVING)
			animate(Animation::RECEIVING);

		else if (ui_state == UI_STATE_WRITING)
			animate(Animation::WRITING);

		else if (ui_state == UI_STATE_WAITING)
			animate(Animation::WAITING);

		else // if (ui_state == UI_STATE_DONE)
		{}
	}

	void new_block() {
		decoder.Reset();
#ifdef USING_FSK
		demodulator.Sync(); // FSK
#else
		demodulator.SyncCarrier(false); // QPSK
#endif
	}

	void new_packet() {
#ifdef USING_FSK
		decoder.Reset(); // FSK
#else
		decoder.Reset();
		demodulator.SyncDecision(); // QPSK
#endif
	}

	void animate_until_button_pushed(Animation animation_type, Button button) {
		animate(Animation::RESET);

		while (!button_pushed(button)) {
			HAL_Delay(1);
			animate(animation_type);
		}
		while (button_pushed(button)) {
			HAL_Delay(1);
		}
	}
};

} // namespace LDKit::Bootloader
