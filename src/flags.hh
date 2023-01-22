#pragma once
#include <cstdint>

namespace LDKit
{
struct Flags {
private:
	// TODO: make these atomic, if necessary
	bool _time_changed = true;
	bool _inf_changed = false;
	bool _rev_changed = false;

public:
	uint32_t mute_on_boot_ctr = 12000;

	bool time_changed() {
		auto t = _time_changed;
		_time_changed = false;
		return t;
	}
	bool inf_changed() {
		auto t = _inf_changed;
		_inf_changed = false;
		return t;
	}
	bool rev_changed() {
		auto t = _rev_changed;
		_rev_changed = false;
		return t;
	}
	void set_time_changed() { _time_changed = true; }
	void set_inf_changed() { _inf_changed = true; }
	void set_rev_changed() { _rev_changed = true; }
};
} // namespace LDKit
