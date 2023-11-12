#include "cowdata.h"

template <typename T>
void lain::CowData<T>::_Ref(const CowData<T> p_from){
	if (m_ptr == p_from.m_ptr) {
		return;
	}


}

template <typename T>
void lain::CowData<T>::_CopyOnWrite() {
	if (m_ptr->get_refcount() == 1) {
		return;
	}

	// copy data
	u32* count = _GetSize();
	T* data = (T*)(count + 1);
	T* new_data = (T*)Memory::alloc_static(*count * sizeof(T), true);
	for (uint32_t i = 0; i < *count; ++i) {
		new_data[i] = data[i];
	}

	// unref old data
	_Unref(m_ptr);

	// set new data
	m_ptr = (CowData<T>)new_data;
}
template <typename T>

void lain::CowData<T>::_Unref(const CowData<T> p_data) {
	if (!p_data) {
		return;
	}
	s_u32* refcount = _GetCount(p_data);
	if (refcount->decrement()>0) {
		return;
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
