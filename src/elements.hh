#pragma once

#include <cstdint>

namespace LDKit
{

enum AdcElement : uint32_t {
	TimePot,
	FeedbackPot,
	DelayFeedPot,
	MixPot,
	TimeCV,
	FeedbackCV,
	DelayFeedCV,
	MixCV,
};

constexpr static uint32_t NumAdcs = 8;

enum TrigInJackElement : uint32_t {
	PingJack,
};

} // namespace LDKit
