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

		L_INLINE const T* _ptrw()const {
			_CopyOnWrite();
			return m_ptr;
		}


		L_INLINE u32* _GetSize() const {
			if (!ptr) {
				return nullptr;
			}
			return reinterpret_cast<u32*>(ptr) - 1;
		}
		L_INLINE s_u32* _GetCount() const {
			if (!ptr) {
				return nullptr;
			}
			return reinterpret_cast<s_u32*>(ptr) - 2;
		}
		L_INLINE size_t _GetAllocSize(size_t element)const {
			return next_power_of_2(element * sizeof(T));
		}
		void _Ref(const CowData<T>& p_from);
		void _Unref(void* p_data);
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
		template <bool p_ensure_zero>
		Error Resize(int size);
		L_INLINE void Clear() { Resize(0) };
		L_INLINE bool IsEmpty() { return (m_ptr == nullptr); }
		int Find(const T& p_val, int p_from) const;
		L_INLINE const T& Get(int p_index) const {
			CRASH_BAD_INDEX(p_index, Size());
			return m_ptr[p_index];
		}

		L_INLINE const T& Set(int p_index, const T& p_elem) const {
			CRASH_BAD_INDEX(p_index, Size());
			_CopyOnWrite();
			m_ptr[p_index] = p_elem;
		}
		_FORCE_INLINE_ void Remove_at(int p_index) {
			ERR_FAIL_INDEX(p_index, Size());
			T* p = _ptrw();
			int len = Size();
			for (int i = p_index; i < len - 1; i++) {
				p[i] = p[i + 1];
			}

			Resize(len - 1);
		}

		_FORCE_INLINE_ T& Get_W(int p_index) {
			CRASH_BAD_INDEX(p_index, size());
			_CopyOnWrite();
			return m_ptr[p_index];
		}


	};
}

