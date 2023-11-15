#pragma once
#include <core/typedefs.h>
#include <base.h>
#include "core/error/error_macros.h"
#include "core/os/memory.h"
#include <core/templates/safe_refcount.h>
#include <type_traits>
namespace lain {

	template <class T>
	class Vector;

	typedef uint32_t u32;

	template <class T>
	class CowData {
		template<class TV>
		friend class Vector;
	private:
		// structure:
		// memory: (refcount(safe); size(uint) ; data)
		mutable T* m_ptr = nullptr; // pointer to the data
		// always mutable, and could be modified even in const function.


	private:
		L_INLINE const T* _ptr()const {
			return m_ptr;
		}

		L_INLINE T* _ptrw() {
			_CopyOnWrite();
			return m_ptr;
		}


		L_INLINE u32* _GetSize() const {
			if (!m_ptr) {
				return nullptr;
			}
			return reinterpret_cast<u32*>(m_ptr) - 1;
		}
		L_INLINE s_u32* _GetCount() const {
			if (!m_ptr) {
				return nullptr;
			}
			return reinterpret_cast<s_u32*>(m_ptr) - 2;
		}
		L_INLINE size_t _GetAllocSize(size_t element) const {
			return next_power_of_2(element * sizeof(T));
		}
		void _Ref(const CowData<T>& p_from);
		void _Ref(const CowData<T>* p_from) {
			_Ref(*p_from);
		}
		void _Unref(void* p_data);
		u32 _CopyOnWrite();
	public:
		L_INLINE const T* ptr()const {
			return m_ptr;
		}

		L_INLINE T* ptrw() {
			_CopyOnWrite();
			return m_ptr;
		}
		// cow reloaded "="
		void operator =(const CowData<T>& p_from) { _Ref(p_from); }

		L_INLINE CowData() {}
		L_INLINE ~CowData();
		L_INLINE CowData(CowData<T>& p_from) { _Ref(p_from); };
		L_INLINE u32 Size() const {
			u32* size = (u32*)_GetSize();
			if (size != nullptr) {
				return *size;
			}
			return 0;
		}
		template <bool p_ensure_zero = false>
		Error Resize(int size);
		L_INLINE void Clear() { Resize(0) };
		L_INLINE bool IsEmpty()const { return (m_ptr == nullptr); }
		int Find(const T& p_val, int p_from = 0) const;
		int Rfind(const T& p_val, int p_from = -1) const;
		L_INLINE const T& Get(int p_index) const {
			CRASH_BAD_INDEX(p_index, Size());
			return m_ptr[p_index];
		}

		L_INLINE void Set(int p_index, const T& p_elem) {
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

		Error Insert_at(int p_pos, const T& p_val) {
			ERR_FAIL_INDEX_V(p_pos, Size() + 1, ERR_INVALID_PARAMETER);
			Resize(Size() + 1);
			for (int i = (Size() - 1); i > p_pos; i--) {
				Set(i, Get(i - 1));
			}
			Set(p_pos, p_val);

			return OK;
		}
		
		int Count(const T& p_val) const;
		L_INLINE int FindLast(const T& p_val) const;
	};

	// stop reference that data.
	// if not ref,return; else decrease; if rc ==0 call deconstructor.
	template <class T>
	void CowData<T>::_Unref(void* p_data) {
		if (!p_data) {
			return;
		}
		s_u32* refcount = _GetCount();
		if (refcount->decrement() > 0) {
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

	template <class T>
	void CowData<T>::_Ref(const CowData<T>& p_from) {
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
	// cow: if count <= 1, do nothing. else change m_ptr to anther one.
	template <class T>
	u32 CowData<T>::_CopyOnWrite() {
		if (!m_ptr) {
			return 0;
		}

		s_u32* refcount = _GetCount();
		u32 rc = refcount->get();
		if (unlikely(rc > 1)) {
			/* in use by more than me */
			uint32_t current_size = *_GetSize();

			uint32_t* mem_new = (uint32_t*)Memory::alloc_static(_GetAllocSize(current_size), true);
			// placement new
			new (mem_new - 2) s_u32(1); //refcount
			*(mem_new - 1) = current_size; //size

			T* _data = (T*)(mem_new);

			// initialize new elements
			if (std::is_trivially_copyable<T>::value) {
				memcpy(mem_new, m_ptr, current_size * sizeof(T));

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


	template <class T>
	CowData<T>::~CowData()
	{
		_Unref((void*)m_ptr);
	}

	template <class T>
	template <bool p_ensure_zero>
	Error CowData<T>::Resize(int p_size) {
		ERR_FAIL_COND_V(p_size < 0, ERR_INVALID_PARAMETER);
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
		// L_PRINT("resize to", p_size, current_alloc_size,alloc_size);

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
					uint32_t* _ptrnew = (uint32_t*)Memory::realloc_static(m_ptr, alloc_size, true);
					ERR_FAIL_COND_V(!_ptrnew, ERR_OUT_OF_MEMORY);
					new (_ptrnew - 2) s_u32(rc); //refcount

					m_ptr = (T*)(_ptrnew);
				}
			}

			// construct the newly created elements

			if (!std::is_trivially_constructible<T>::value) {
				for (int i = *_GetSize(); i < p_size; i++) {
					memnew_placement(&m_ptr[i], T);
				}
			}
			else if (p_ensure_zero) {
				memset((void*)(m_ptr + current_size), 0, (p_size - current_size) * sizeof(T));
			}

			*_GetSize() = p_size;

		}

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
				// new with placement 
				new (_ptrnew - 2) s_u32(rc); //refcount

				m_ptr = (T*)(_ptrnew);
			}

			*_GetSize() = p_size;
		}

		return OK;
	}

	template <class T>
	int CowData<T>::Find(const T& p_val, int p_from) const {
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
	// find from right to left
	template <class T>
	int CowData<T>::Rfind(const T& p_val, int p_from) const {
		const int s = size();

		if (p_from < 0) {
			p_from = s + p_from;
		}
		if (p_from < 0 || p_from >= s) {
			p_from = s - 1;
		}

		for (int i = p_from; i >= 0; i--) {
			if (get(i) == p_val) {
				return i;
			}
		}
		return -1;
	}

	template <class T>
	int CowData<T>::FindLast(const T& p_val) const {
		return Rfind(p_val);
	}

	template <class T>
	int CowData<T>::Count(const T& p_val) const {
		int amount = 0;
		for (int i = 0; i < Size(); i++) {
			if (Get(i) == p_val)
				amount++;
		}
		return amount;
	}
}