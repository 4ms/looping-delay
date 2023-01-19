#include <cstdint>

template<float LPFCoef, int32_t MinLevel, float AttackRate, float DecayRate>
class AutoMute {
	enum State { Unmuted, FadingDown, Muted, FadingUp } state;
	float fade_amt;
	float lpf;

public:
	AutoMute() = default;

	int32_t update(int32_t cur) {
		lpf = (lpf * (1.f - LPFCoef)) + (((cur > 0) ? cur : -cur) * LPFCoef);

		// TODO:
		return cur;
	}
};
