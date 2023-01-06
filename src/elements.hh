#pragma once

#include <cstdint>

namespace LoopingDelay
{

enum AdcElement : uint32_t {
	TimePot,
	FeedbackPot,
	MixPot,
	DelayFeedPot,
	TimeCV,
	FeedbackCV,
	MixCV,
	DelayFeedCV,
};

constexpr static uint32_t NumAdcs = 8;

enum TrigInJackElement : uint32_t {
	PingJack,
};

} // namespace LoopingDelay
