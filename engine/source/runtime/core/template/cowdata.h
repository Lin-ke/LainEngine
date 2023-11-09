#pragma once
#include <core/typedefs.h>
#include <base.h>
template <typename T>
class CowData {
private:
	// structure:
	// (32bit refcount; 32bit size; data)
	mutable T* m_ptr = null_ptr; // pointer to the data
	// always mutable, and could be modified even in const function.

public:
	// cow reloaded "="
	void operator =(const CowData<T>& p_from) { _ref(p_from); }
	L_INLINE const T* ptr() {
		return m_ptr;
	}


	L_INLINE size32_t* size();
};