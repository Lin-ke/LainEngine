#include "memory.h"

void* operator new(size_t p_size, const char* p_description) {
	return Memory::alloc_static(p_size, false);
}

void* operator new(size_t p_size, void* (*p_allocfunc)(size_t p_size)) {
	return p_allocfunc(p_size);
}

#ifdef _MSC_VER
void operator delete(void* p_mem, const char* p_description) {
	CRASH_NOW_MSG("Call to placement delete should not happen.");
}

void operator delete(void* p_mem, void* (*p_allocfunc)(size_t p_size)) {
	CRASH_NOW_MSG("Call to placement delete should not happen.");
}

void operator delete(void* p_mem, void* p_pointer, size_t check, const char* p_description) {
	CRASH_NOW_MSG("Call to placement delete should not happen.");
}
#endif
// initialize mem_usage
#ifdef L_DEBUG
SafeNumeric<uint64_t> Memory::mem_usage;
SafeNumeric<uint64_t> Memory::max_usage;
#endif

// memory management
void* Memory::alloc_static(size_t p_bytes, bool p_pad_align) {
#ifdef L_DEBUG // always prepad
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

void* Memory::realloc_static(void* p_memory, size_t p_bytes, bool p_pad_align) {
	if (p_memory == nullptr) {
		return alloc_static(p_bytes, p_pad_align);
	}
#ifdef L_DEBUG // always prepad
	bool prepad = true;
#else
	bool prepad = p_pad_align;
#endif
	uint8_t* mem = (uint8_t*)p_memory;
	if (prepad) { 
		mem -= PAD_ALIGN;
		uint64_t* s = (uint64_t*)mem;

#ifdef L_DEBUG
		if (p_bytes > *s) {
			uint64_t new_mem_usage = mem_usage.add(p_bytes - *s);
			max_usage.exchange_if_greater(new_mem_usage);
		}
		else {
			mem_usage.sub(*s - p_bytes);
		}
#endif

		if (p_bytes == 0) {
			free(mem);
			return nullptr;
		}
		else {
			*s = p_bytes;

			mem = (uint8_t*)realloc(mem, p_bytes + PAD_ALIGN);
			ERR_FAIL_NULL_V(mem, nullptr);

			s = (uint64_t*)mem;

			*s = p_bytes;

			return mem + PAD_ALIGN;
		}
	}
	else {
		mem = (uint8_t*)realloc(mem, p_bytes);

		ERR_FAIL_COND_V(mem == nullptr && p_bytes > 0, nullptr);

		return mem;
	}
}

void Memory::free_static(void* p_ptr, bool p_pad_align) {
	ERR_FAIL_NULL(p_ptr);

	uint8_t* mem = (uint8_t*)p_ptr;

#ifdef L_DEBUG
	bool prepad = true;
#else
	bool prepad = p_pad_align;
#endif

	alloc_count.decrement();

	if (prepad) {
		mem -= PAD_ALIGN;

#ifdef L_DEBUG
		uint64_t* s = (uint64_t*)mem;
		mem_usage.sub(*s);
#endif

		free(mem);
	}
	else {
		free(mem);
	}
}

uint64_t Memory::get_mem_available() {
	return -1; // 0xFFFF...
}

uint64_t Memory::get_mem_usage() {
#ifdef L_DEBUG
	return mem_usage.get();
#else
	return 0;
#endif
}

uint64_t Memory::get_mem_max_usage() {
#ifdef L_DEBUG
	return max_usage.get();
#else
	return 0;
#endif
}

_GlobalNil::_GlobalNil() {
	left = this;
	right = this;
	parent = this;
}

_GlobalNil _GlobalNilClass::_nil;
