#pragma once
#include <core/typedefs.h>
#include <base.h>
#include <core/template/saferefcount.h>
#include <vector>
// 成员变量 首字母标识
// 函数：大驼峰
//
template <typename T>
class CowData {
private:
	// structure:
	// memory: (32bit refcount; 32bit size; data)
	mutable T* m_ptr = null_ptr; // pointer to the data
	// always mutable, and could be modified even in const function.

public:
	// cow reloaded "="
	void operator =(const CowData<T>& p_from) { _ref(p_from); }
private:
	L_INLINE const T* _ptr()const {
		return m_ptr;
	}


	L_INLINE s_u32* _GetSize() const {
		if (!ptr) {
			return nullptr;
		}
		return reinterpret_cast<s_u32*>(ptr) - 1;
	}
	L_INLINE s_u32* _GetCount() const {
		if (!ptr) {
			return nullptr;
		}
		return reinterpret_cast<s_u32*>(ptr) - 1;
	}
	L_INLINE size_t _GetAllocSize(size_t element)const {
		return 
	}
};