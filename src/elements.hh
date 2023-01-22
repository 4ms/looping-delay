#pragma once

#include <cstdint>

namespace LDKit
{

enum PotAdcElement : uint32_t {
	TimePot,
	FeedbackPot,
	DelayFeedPot,
	MixPot,
};

enum CVAdcElement : uint32_t {
	TimeCV,
	FeedbackCV,
	DelayFeedCV,
	MixCV,
};

constexpr static uint32_t NumPots = 4;
constexpr static uint32_t NumCVs = 4;
constexpr static uint32_t NumAdcs = NumPots + NumCVs;

enum TrigInJackElement : uint32_t {
	PingJack,
};

} // namespace LDKit
