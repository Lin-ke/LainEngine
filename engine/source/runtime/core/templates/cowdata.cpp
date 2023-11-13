
#include "cowdata.h"
#include "core/error/error_macros.h"
#include "core/os/memory.h"
template <typename T>
void lain::CowData<T>::_Ref(const CowData<T>& p_from){
	if (m_ptr == p_from.m_ptr) {
		return;
	}
	_Unref(m_ptr);
	m_ptr = nullptr;
	if (!p_from.m_ptr) {
		return;
	}
	// increment refcount
	if (p_from._GetCount()->conditional_increment() > 0) {
		m_ptr = p_from.m_ptr;
	}
	

}
// cow: change the m_ptr pointer.
template <typename T>
void lain::CowData<T>::_CopyOnWrite() {
	if (!m_ptr) {
		return 0; 
	}
	
	s_u32* refcount = _GetCount();
	u32 rc = refcount->get();
	if (unlikely(rc > 1)) {
		/* in use by more than me */
		uint32_t current_size = *_GetSize();

		uint32_t* mem_new = (uint32_t*)Memory::alloc_static(_GetAllocSize(current_size), true);

		new (mem_new - 2) s_u32(1); //refcount
		*(mem_new - 1) = current_size; //size

		T* _data = (T*)(mem_new);

		// initialize new elements
		if (std::is_trivially_copyable<T>::value) {
			memcpy(mem_new, _ptr, current_size * sizeof(T));

		}
		else {
			for (uint32_t i = 0; i < current_size; i++) {
				memnew_placement(&_data[i], T(m_ptr[i]));
			}
		}

		_Unref(m_ptr);
		m_ptr = _data;

		rc = 1;
	}
	return rc; // return 1.
}

// stop reference that data.
template <typename T>

void lain::CowData<T>::_Unref(void* p_data) {
	if (!p_data) {
		return;
	}
	s_u32* refcount = _GetCount();
	if (refcount->decrement()>0) {
		return; // refed by others.
	}
	// GC: refcount ==0

	// call destructors
	if (!std::is_trivially_destructible<T>::value) {
		u32* count = _GetSize();
		T* data = (T*)(count + 1);

		for (uint32_t i = 0; i < *count; ++i) {
			// call destructors
			data[i].~T();
		}
	}

	// free mem
	Memory::free_static((uint8_t*)p_data, true);
	
}
template <typename T>
template <bool p_ensure_zero>

Error lain::CowData<T>::Resize(int p_size) {
	ERR_FAIL_COND_V(size < 0, ERR_INVALID_PARAMETER);
	int current_size = Size();

	if (p_size == current_size) {
		return OK;
	}

	if (p_size == 0) {
		// wants to clean up
		_Unref(m_ptr);
		m_ptr = nullptr;
		return OK;
	}

	// possibly changing size, copy on write
	uint32_t rc = _CopyOnWrite();

	size_t current_alloc_size = _GetAllocSize(current_size);
	size_t alloc_size = _GetAllocSize(p_size);
	// ERR_FAIL_COND_V(!_get_alloc_size_checked(p_size, &alloc_size), ERR_OUT_OF_MEMORY);

	if (p_size > current_size) {
		if (alloc_size != current_alloc_size) {
			if (current_size == 0) {
				// alloc from scratch
				uint32_t* ptr = (uint32_t*)Memory::alloc_static(alloc_size, true);
				ERR_FAIL_COND_V(!ptr, ERR_OUT_OF_MEMORY);
				*(ptr - 1) = 0; //size, currently none
				new (ptr - 2) s_u32(1); //refcount

				m_ptr = (T*)ptr;

			}
			else {
				uint32_t* _ptrnew = (uint32_t*)Memory::realloc_static(_ptr, alloc_size, true);
				ERR_FAIL_COND_V(!_ptrnew, ERR_OUT_OF_MEMORY);
				new (_ptrnew - 2) s_u32(rc); //refcount

				m_ptr = (T*)(_ptrnew);
			}
		}

		// construct the newly created elements

		if (!std::is_trivially_constructible<T>::value) {
			for (int i = *_GetSize(); i < p_size; i++) {
				memnew_placement(&_ptr[i], T);
			}
		}
		else if (p_ensure_zero) {
			memset((void*)(_ptr + current_size), 0, (p_size - current_size) * sizeof(T));
		}

		*_GetSize() = p_size;

	}

	// cut
	else if (p_size < current_size) {
		if (!std::is_trivially_destructible<T>::value) {
			// deinitialize no longer needed elements
			for (uint32_t i = p_size; i < *_GetSize(); i++) {
				T* t = &m_ptr[i];
				t->~T();
			}
		}

		if (alloc_size != current_alloc_size) {
			uint32_t* _ptrnew = (uint32_t*)Memory::realloc_static(m_ptr, alloc_size, true);
			ERR_FAIL_COND_V(!_ptrnew, ERR_OUT_OF_MEMORY);
			new (_ptrnew - 2) s_u32(rc); //refcount

			m_ptr = (T*)(_ptrnew);
		}

		*_get_size() = p_size;
	}

	return OK;
}

template <class T>
int lain::CowData<T>::Find(const T& p_val, int p_from) const {
	int ret = -1;

	if (p_from < 0 || size() == 0) {
		return ret;
	}

	for (int i = p_from; i < size(); i++) {
		if (Get(i) == p_val) {
			ret = i;
			break;
		}
	}

	return ret;
}

