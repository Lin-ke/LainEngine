#pragma once
#ifndef __safe_numeric__
#define __safe_numeric__
#include <core/typedefs.h>
#include <base.h>
#include <atomic>
// This class is used to store a thread safe reference count, using c++11 atomic.
template <typename T>
class SafeNumeric {
	private:
	std::atomic<T> value;
	// atomic operations:
	// store, load, exchange, compare_exchange_weak, compare_exchange_strong, fetch_add, fetch_sub, fetch_or, fetch_and, fetch_xor
	// atomic has 6 relations
	//enum memory_order {
	//	memory_order_relaxed,
	//	memory_order_consume,
	//	memory_order_acquire,
	//	memory_order_release,
	//	memory_order_acq_rel,
	//	memory_order_seq_cst
	//};
	// memory_order_relaxed: no order
	// memory_order_consume: A load operation. no reads or writes in the current thread *dependent on* the value currently loaded can be reordered before this load.
	// memory_order_acquire: A load operation. no reads or writes in the current thread can be reordered before this load. 
	// memory_order_release: A store operation. no reads or writes in the current thread can be reordered after this store.All writes in the current thread are visible in other threads that acquire the same atomic variable, and writes that carry a dependency into the atomic variable become visible in other threads that *consume* the same atomic .
	// memory_order_acq_rel: A read-modify-write operation with this memory order is both an acquire operation and a release operation. No memory reads or writes in the current thread can be reordered before the load, nor after the store. All writes in other threads that release the same atomic variable are visible before the modification and the modification is visible in other threads that acquire the same atomic variable. 
	// memory_order_seq_cst: A load operation with this memory order performs an acquire operation, a store performs a release operation, and read-modify-write performs both an acquire operation and a release operation, plus a single total order exists in which all threads observe all modifications in the same order

public :
	L_INLINE void set(T p_value) {
		value.store(p_value, std::memory_order_release);
	}

	L_INLINE T get() const {
		return value.load(std::memory_order_acquire);
	}

	L_INLINE T increment() {
		return value.fetch_add(1, std::memory_order_acq_rel) + 1;
	}

	// Returns the original value instead of the new one
	L_INLINE T postincrement() {
		return value.fetch_add(1, std::memory_order_acq_rel);
	}

	L_INLINE T decrement() {
		return value.fetch_sub(1, std::memory_order_acq_rel) - 1;
	}

	// Returns the original value instead of the new one
	L_INLINE T postdecrement() {
		return value.fetch_sub(1, std::memory_order_acq_rel);
	}

	L_INLINE T add(T p_value) {
		return value.fetch_add(p_value, std::memory_order_acq_rel) + p_value;
	}

	// Returns the original value instead of the new one
	L_INLINE T postadd(T p_value) {
		return value.fetch_add(p_value, std::memory_order_acq_rel);
	}

	L_INLINE T sub(T p_value) {
		return value.fetch_sub(p_value, std::memory_order_acq_rel) - p_value;
	}

	L_INLINE T bit_or(T p_value) {
		return value.fetch_or(p_value, std::memory_order_acq_rel);
	}
	L_INLINE T bit_and(T p_value) {
		return value.fetch_and(p_value, std::memory_order_acq_rel);
	}

	L_INLINE T bit_xor(T p_value) {
		return value.fetch_xor(p_value, std::memory_order_acq_rel);
	}

	// Returns the original value instead of the new one
	L_INLINE T postsub(T p_value) {
		return value.fetch_sub(p_value, std::memory_order_acq_rel);
	}

	L_INLINE T exchange_if_greater(T p_value) {
		while (true) {
			T tmp = value.load(std::memory_order_acquire);
			if (tmp >= p_value) {
				return tmp; // already greater, or equal
			}

			if (value.compare_exchange_weak(tmp, p_value, std::memory_order_acq_rel)) {
				return p_value;
			}
		}
	}

	L_INLINE T conditional_increment() {
		while (true) {
			T c = value.load(std::memory_order_acquire);
			if (c == 0) {
				return 0;
			}
			if (value.compare_exchange_weak(c, c + 1, std::memory_order_acq_rel)) {
				return c + 1;
			}
		}
	}

	L_INLINE explicit SafeNumeric<T>(T p_value = static_cast<T>(0)) {
		set(p_value);
	}
	
};
typedef SafeNumeric<uint32_t> s_u32;
typedef SafeNumeric<uint64_t> s_u64;
#endif // __safe_numeric__