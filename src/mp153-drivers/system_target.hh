#pragma once
#include "brain_conf.hh"
#include "ld.h"
#include "printf.h"

namespace LDKit
{
struct SystemTarget {
	static void init() {

		if (A7_HEAP != Brain::MemoryStartAddr) {
			printf_("WARNING: linker starts A7_HEAP at %ld but Brain::MemoryStartAddr is %ld\n",
					A7_HEAP,
					Brain::MemoryStartAddr);
		}
		if (A7_HEAP_SZ != Brain::MemorySizeBytes) {
			printf_("WARNING: linker starts A7_HEAP at %ld but Brain::MemoryStartAddr is %ld\n",
					A7_HEAP,
					Brain::MemoryStartAddr);
		}
	}

	static void restart() {
		// TODO
	}
};
} // namespace LDKit
