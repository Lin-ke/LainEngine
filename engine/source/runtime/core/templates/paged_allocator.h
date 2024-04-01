#pragma once
#ifndef PAGED_ALLOCATOR_H
#define PAGED_ALLOCATOR_H

#include "core/os/memory.h"
#include "core/os/spin_lock.h"
#include "core/string/ustring.h"
#include "core/typedefs.h"

#include <type_traits>
#include <typeinfo>
namespace lain {
	// 一页一页的分配内存，每页pagesize大小。
	// 每次分配一个available指向可用的
template <class T, bool thread_safe = false, uint32_t DEFAULT_PAGE_SIZE = 4096>
class PagedAllocator {
	T** page_pool = nullptr;
	T*** available_pool = nullptr;

	uint32_t pages_allocated = 0;
	uint32_t allocs_available = 0;

	// configure according to size
	uint32_t page_shift = 0;
	uint32_t page_mask = 0;
	uint32_t page_size = 0;
	SpinLock spin_lock;
	
public:
	enum {
		DEFAULT_PAGE_SIZE = 4096
	};
	template <class... Args>
	T* alloc(const Args &&...p_args) {
		if (thread_safe) {
			spin_lock.lock();
		}
		if (unlikely(allocs_available == 0)) {
			uint32_t pages_used = pages_allocated;

			pages_allocated++;
			// 页面池是一个指针数组(T**)，这些指针指向T的所在内存(二维数组，列是索引）
			page_pool = (T**)memrealloc(page_pool, sizeof(T*) * pages_allocated);
			//available_pool = (T***)memrealloc(available_pool, sizeof(T**) * pages_allocated);
			// 分配新页面的内存空间
			page_pool[pages_used] = (T*)memalloc(sizeof(T) * page_size);
			//available_pool[pages_used] = (T**)memalloc(sizeof(T*) * page_size);

			// 将新页面的对象指针添加到可用对象指针数组中
			for (uint32_t i = 0; i < page_size; i++) {
				available_pool[0][i] = &page_pool[pages_used][i]; // 因为这个时候available是空的，所以放在第一
			}
			// 增加可用对象计数
			allocs_available += page_size; 
		}

		allocs_available--;
		// 行索引位数（高位）和列索引位数（低位）
		T* alloc = available_pool[allocs_available >> page_shift][allocs_available & page_mask];

		if (thread_safe) {
			spin_lock.unlock();
		}
		memnew_placement(alloc, T(p_args...));
		return alloc;
	}

	void free(T* p_mem) {
		if (thread_safe) {
			spin_lock.lock();
		}
		p_mem->~T();
		available_pool[allocs_available >> page_shift][allocs_available & page_mask] = p_mem;
		allocs_available++;
		if (thread_safe) {
			spin_lock.unlock();
		}
	}
	template<class... Args>
	T* new_allocation(Args &&...p_args) { return alloc(p_args...); }
	void delete_allocation(T* p_mem) { free(p_mem); }
private:
	void _reset(bool p_allow_unfreed) {
		if (!p_allow_unfreed || !std::is_trivially_destructible<T>::value) {
			ERR_FAIL_COND(allocs_available < pages_allocated * page_size);
		}
		if (pages_allocated) {
			for (uint32_t i = 0; i < pages_allocated; i++) {
				memfree(page_pool[i]);
				memfree(available_pool[i]);
			}
			memfree(page_pool);
			memfree(available_pool);
			page_pool = nullptr;
			available_pool = nullptr;
			pages_allocated = 0;
			allocs_available = 0;
		}
	}

public:
	void reset(bool p_allow_unfreed = false) {
		if (thread_safe) {
			spin_lock.lock();
		}
		_reset(p_allow_unfreed);
		if (thread_safe) {
			spin_lock.unlock();
		}
	}

	bool is_configured() const {
		if (thread_safe) {
			spin_lock.lock();
		}
		bool result = page_size > 0;
		if (thread_safe) {
			spin_lock.unlock();
		}
		return result;
	}
	// 在这里确定索引位数
	void configure(uint32_t p_page_size) {
		if (thread_safe) {
			spin_lock.lock();
		}
		ERR_FAIL_COND(page_pool != nullptr); //sanity check
		ERR_FAIL_COND(p_page_size == 0);
		// default: 4096
		page_size = nearest_power_of_2_templated(p_page_size);

		page_mask = page_size - 1;
		page_shift = get_shift_from_power_of_2(page_size);
		if (thread_safe) {
			spin_lock.unlock();
		}
	}

	// Power of 2 recommended because of alignment with OS page sizes.
	// Even if element is bigger, it's still a multiple and gets rounded to amount of pages.
	PagedAllocator(uint32_t p_page_size = DEFAULT_PAGE_SIZE) {
		configure(p_page_size);
	}

	~PagedAllocator() {
		if (thread_safe) {
			spin_lock.lock();
		}
		bool leaked = allocs_available < pages_allocated * page_size;
		if (leaked) {
			//if (leak_reporting_enabled) {
				L_CORE_WARN(String("Pages in use exist at exit in PagedAllocator: ") + String(typeid(T).name()));
			//}
		}
		else {
			_reset(false);
		}
		if (thread_safe) {
			spin_lock.unlock();
		}
	}
};
}

#endif // PAGED_ALLOCATOR_H
