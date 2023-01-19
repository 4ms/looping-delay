#include <cstdint>

template<float Coef>
class LpfIir {
	float lpf = 0.f;

public:
	LpfIir() = default;

	int32_t update(int32_t cur) {
		lpf = (lpf * (1.f - Coef)) + (cur * Coef);
		return lpf;
	}
};
