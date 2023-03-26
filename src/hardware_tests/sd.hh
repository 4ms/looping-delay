#pragma once
#include "conf/board_conf.hh"
#include "conf/sd_conf.hh"
#include "drivers/sdcard.hh"
#include "fatfs/fat_file_io.hh"
#include "fatfs/sdcard_ops.hh"
#include "hardware_tests/util.hh"
#include "printf.h"
#include "util/zip.hh"

namespace SamplerKit::HWTests
{

struct TestSDCard {
	SDCardOps<Brain::SDCardConf> sdcard_ops;
	FatFileIO sdcard{&sdcard_ops, Volume::SDCard};

	void run_fatfs_test() {
		std::string_view wbuf{"This is a \ntest of writing\n to a file."};
		bool ok = false;

		while (!ok) {
			Board::RevLED{}.set_color(Colors::off);
			Board::PlayLED{}.set_color(Colors::off);
			Board::BankLED{}.set_color(Colors::off);

			Board::RevB::set(true);
			printf_("Attempting to mount SD Card\n");
			ok = sdcard.mount_disk();
			if (!ok) {
				printf_("Failed to mount SD Card\n");
				Board::RevR::set(true);
				Util::flash_mainbut_until_pressed();
				continue;
			}

			Board::RevG::set(true);
			printf_("Attempting to write file\n");
			ok = sdcard.create_file("test.txt", wbuf);
			if (!ok) {
				printf_("Failed to create file test.txt\n");
				Board::RevR::set(true);
				Util::flash_mainbut_until_pressed();
				continue;
			}

			std::array<char, 128> rbuf;
			Board::BankG::set(true);
			printf_("Attempting to read file\n");
			auto bytes_read = sdcard.read_file("test.txt", rbuf);
			if (!bytes_read || bytes_read != wbuf.size()) {
				printf_("Failed to read file test.txt. Bytes_read = %d\n", bytes_read);
				ok = false;
				Board::BankR::set(true);
				Util::flash_mainbut_until_pressed();
				continue;
			}

			for (auto [w, r] : zip(wbuf, rbuf)) {
				if (w != r) {
					printf_("File was not the same when read back.\n");
					ok = false;
					Board::RevB::set(true);
					Board::BankR::set(true);
					Util::flash_mainbut_until_pressed();
					continue;
				}
			}
		}

		// sdcard.foreach_file_with_ext(".wav", [&](std::string_view fname, uint32_t timestamp, uint32_t fsize) {
		// 	printf_("%.64s %08x %dB\n", fname.data(), timestamp, fsize);
		// 	// action(fno.fname, timestamp, fno.fsize);
		// });
	}

	void run_test() {
		constexpr uint32_t block_to_test = 1024;

		auto &sd = sdcard_ops.sd;
		// mdrivlib::SDCard<Brain::SDCardConf> sd;

		// Allow user to insert a card if they forgot to
		// or try a different card
		bool ok = false;
		while (!ok) {
			Board::RevLED{}.set_color(Colors::off);
			Board::PlayLED{}.set_color(Colors::off);
			Board::BankLED{}.set_color(Colors::off);

			// Read into buf, check if canary values were overwritten
			std::array<uint8_t, 512> buf;
			for (auto &x : buf)
				x = 0xAA;

			ok = sd.read(buf, block_to_test);
			if (!ok) {
				Board::RevR::set(true);
				Util::flash_mainbut_until_pressed();
				continue;
			}

			unsigned aas = 0;
			for (auto &x : buf) {
				if (x == 0xAA)
					aas++;
			}
			if (aas > 256) {
				ok = false;
				Board::BankR::set(true);
				Util::flash_mainbut_until_pressed();
				continue;
			}

			// Write calculated values from wbuf
			std::array<uint8_t, block_to_test> wbuf;
			for (unsigned i = 0; auto &x : wbuf) {
				x = (i * 17) & 0xFF;
				i++;
			}
			ok = sd.write(wbuf, block_to_test);
			if (!ok) {
				Board::BankB::set(true);
				Util::flash_mainbut_until_pressed();
				continue;
			}
			// clear wbuf
			for (auto &x : wbuf)
				x = 0;

			// Read back and compare
			ok = sd.read(wbuf, block_to_test);
			for (unsigned i = 0; auto &x : wbuf) {
				if (x != ((i * 17) & 0xFF))
					ok = false;
				i++;
			}
			if (!ok) {
				Board::BankB::set(true);
				Board::RevR::set(true);
				Util::flash_mainbut_until_pressed();
				continue;
			}

			// Restore original data
			sd.write(buf, block_to_test);
		}
	}
};

} // namespace SamplerKit::HWTests
