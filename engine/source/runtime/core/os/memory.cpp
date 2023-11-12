#include "memory.h"

// memory management
void* Memory::alloc_static(size_t p_bytes, bool p_pad_align) {
#ifdef L_DEBUG
	bool prepad = true;
#else
	bool prepad = p_pad_align;
#endif

	void* mem = malloc(p_bytes + (prepad ? PAD_ALIGN : 0));

	ERR_FAIL_COND_V(!mem, nullptr);

	alloc_count.increment();

	if (prepad) {
		uint64_t* s = (uint64_t*)mem;
		*s = p_bytes;

		uint8_t* s8 = (uint8_t*)mem;

#ifdef L_DEBUG
		uint64_t new_mem_usage = mem_usage.add(p_bytes);
		max_usage.exchange_if_greater(new_mem_usage);
#endif
		return s8 + PAD_ALIGN;
	}
	else {
		return mem;
	}
}