#pragma once
#include <cstdint>
namespace SamplerKit::Bootloader
{

enum class Animation { WAITING, WRITING, RECEIVING, SYNC, DONE, SUCCESS, FAIL_ERR, FAIL_SYNC, FAIL_CRC, RESET };

void animate(Animation animation_type);
void animate_signal(int32_t peak);
} // namespace SamplerKit::Bootloader
