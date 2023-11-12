#pragma once
#include <core/typedefs.h>
#include <base.h>
#include <core/templates/safe_refcount.h>
#include <vector>


namespace lain{

	template <typename T>
	class CowData {
		typedef uint32_t u32;
	private:
		// structure:
		// memory: (refcount(safe); size(uint) ; data)
		mutable T* m_ptr = null_ptr; // pointer to the data
		// always mutable, and could be modified even in const function.


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
		L_INLINE u32* _GetCount() const {
			if (!ptr) {
				return nullptr;
			}
			return reinterpret_cast<u32*>(ptr) - 1;
		}
		L_INLINE size_t _GetAllocSize(size_t element)const {
			return
		}
		void _Ref(const CowData<T> p_from);
		void _Unref(const CowData<T> p_data);
		void _CopyOnWrite();
	public:
		// cow reloaded "="
		void operator =(const CowData<T>& p_from) { _Ref(p_from); }

		L_INLINE CowData() {}
		L_INLINE ~CowData() { _Unref(m_ptr); }
		L_INLINE CowData(CowData<T>& p_from) { _Ref(p_from); };
		L_INLINE u32 Size() {
			u32* size = (u32*)_GetSize();
			if (size) {
				return *size;
			}
			return 0;
		}
		L_INLINE void Resize(int size);
		L_INLINE void Clear() { Resize(0) };
		L_INLINE bool IsEmpty() { return (m_ptr == nullptr); }


	};
}

