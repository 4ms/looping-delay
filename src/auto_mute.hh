#include <cmath>
#include <cstdint>

// TODO: take an average of abs(cur)
template<float LPFCoef, int32_t MinLevel, float AttackRate, float DecayRate>
class AutoMute {
	enum State { Unmuted, FadingDown, Muted, FadingUp } state = Unmuted;
	float fade_amt = 1.f;
	float lpf = MinLevel;

public:
	AutoMute() = default;

	int32_t update(int32_t cur) {
		lpf = (lpf * (1.f - LPFCoef)) + (std::abs(cur) * LPFCoef);

		if (lpf < MinLevel && state != Muted)
			state = FadingDown;

		if (lpf >= MinLevel && state != Unmuted)
			state = FadingUp;

		switch (state) {
			case Unmuted:
				return cur;
				break;

			case FadingDown:
				fade_amt -= DecayRate;
				if (fade_amt <= 0.f) {
					fade_amt = 0.f;
					state = Muted;
				}
				break;

			case Muted:
				return 0.f;
				break;

			case FadingUp:
				fade_amt += AttackRate;
				if (fade_amt >= 1.f) {
					fade_amt = 1.f;
					state = Unmuted;
				}
				break;
		}

		return cur * fade_amt;
	}
};
