#pragma once
#include "conf/board_conf.hh"
#include <array>
#include <cstdint>

namespace LDKit
{

using DelayBuffer = std::array<int16_t, 1024 * 1024 * 8>;

static inline DelayBuffer get_delay_buffer() {
	DelayBuffer::value_type *p = reinterpret_cast<DelayBuffer::value_type *>(Board::ExternalMemoryStartAddr);
	DelayBuffer *arr = new (p) DelayBuffer;
	return (*arr);
}

} // namespace LDKit
