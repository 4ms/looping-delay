#include <cstdint>

template<float Coef>
class DCBlock {
	float lpf = 0.f;

public:
	DCBlock() = default;

	int32_t update(int32_t cur) {
		lpf = (lpf * (1.f - Coef)) + (cur * Coef);
		return cur - lpf;
	}
};
