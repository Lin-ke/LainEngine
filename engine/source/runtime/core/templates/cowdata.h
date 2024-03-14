#pragma once
#ifndef __COWDATA_H__
#define __COWDATA_H__
#include <core/typedefs.h>
#include <base.h>
#include "core/error/error_macros.h"
#include "core/os/memory.h"
#include <core/templates/safe_numeric.h>
#include <type_traits>
namespace lain {
	class CharString;
	class Char16String;
	template <class T>
	class Vector;
	class String;

	template <class T>
	class CowData {
		friend class CharString;
		friend class Char16String;

		template<class TV>
		friend class Vector;
		friend class String;
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
			_copy_on_write();
			return m_ptr;
		}


		L_INLINE ui32* _get_size() const {
			if (!m_ptr) {
				return nullptr;
			}
			return reinterpret_cast<ui32*>(m_ptr) - 1;
		}
		L_INLINE s_ui32* _get_count() const {
			if (!m_ptr) {
				return nullptr;
			}
			return reinterpret_cast<s_ui32*>(m_ptr) - 2;
		}
		// 最大支持
		L_INLINE size_t _get_alloc_size(size_t element) const {
			return next_power_of_2(static_cast<ui32> (element * sizeof(T)));
		}
		void _ref(const CowData<T>& p_from);
		void _ref(const CowData<T>* p_from) {
			_ref(*p_from);
		}
		void _unref(void* p_data);
		ui32 _copy_on_write();
	public:
		L_INLINE const T* ptr()const {
			return m_ptr;
		}

		L_INLINE T* ptrw() {
			_copy_on_write();
			return m_ptr;
		}
		// cow reloaded "="
		void operator =(const CowData<T>& p_from) { _ref(p_from); }

		L_INLINE CowData() {}
		L_INLINE ~CowData();
		L_INLINE CowData(CowData<T>& p_from) { _ref(p_from); };
		L_INLINE ui32 size() const {
			ui32* size = (ui32*)_get_size();
			if (size != nullptr) {
				return *size;
			}
			return 0;
		}
		template <bool p_ensure_zero = false>
		Error resize(int size);

		L_INLINE void clear() { resize(0) };
		L_INLINE bool is_empty()const { return (m_ptr == nullptr); }
		int Find(const T& p_val, int p_from = 0) const;
		int Rfind(const T& p_val, int p_from = -1) const;
		L_INLINE const T& get(int p_index) const {
			CRASH_BAD_INDEX(p_index, size());
			return m_ptr[p_index];
		}

		L_INLINE void set(int p_index, const T& p_elem) {
			CRASH_BAD_INDEX(p_index, size());
			_copy_on_write();
			m_ptr[p_index] = p_elem;
		}

		_FORCE_INLINE_ void remove_at(int p_index) {
			ERR_FAIL_INDEX(p_index, size());
			T* p = _ptrw();
			int len = size();
			for (int i = p_index; i < len - 1; i++) {
				p[i] = p[i + 1];
			}

			resize(len - 1);
		}

		_FORCE_INLINE_ T& get_m(int p_index) {
			CRASH_BAD_INDEX(p_index, size());
			_copy_on_write();
			return m_ptr[p_index];
		}

		Error insert_at(int p_pos, const T& p_val) {
			ERR_FAIL_INDEX_V(p_pos, size() + 1, ERR_INVALID_PARAMETER);
			resize(size() + 1);
			for (int i = (size() - 1); i > p_pos; i--) {
				set(i, get(i - 1));
			}
			set(p_pos, p_val);

			return OK;
		}
		
		int count(const T& p_val) const;
		L_INLINE int FindLast(const T& p_val) const;
	};

	// stop reference that data.
	// if not ref,return; else decrease; if rc ==0 call deconstructor.
	template <class T>
	void CowData<T>::_unref(void* p_data) {
		if (!p_data) {
			return;
		}
		s_ui32* refcount = _get_count();
		if (refcount->decrement() > 0) {
			return; // refed by others.
		}
		// GC: refcount ==0

		// call destructors
		if (!std::is_trivially_destructible<T>::value) {
			ui32* count = _get_size();
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
	void CowData<T>::_ref(const CowData<T>& p_from) {
		if (m_ptr == p_from.m_ptr) {
			return;
		}
		_unref(m_ptr);
		m_ptr = nullptr;
		if (!p_from.m_ptr) {
			return;
		}
		// increment refcount
		if (p_from._get_count()->conditional_increment() > 0) {
			m_ptr = p_from.m_ptr;
		}


	}
	// cow: if count <= 1, do nothing. else change m_ptr to anther one.
	template <class T>
	ui32 CowData<T>::_copy_on_write() {
		if (!m_ptr) {
			return 0;
		}

		s_ui32* refcount = _get_count();
		ui32 rc = refcount->get();
		if (unlikely(rc > 1)) {
			/* in use by more than me */
			uint32_t current_size = *_get_size();

			uint32_t* mem_new = (uint32_t*)Memory::alloc_static(_get_alloc_size(current_size), true);
			// placement new
			new (mem_new - 2) s_ui32(1); //refcount
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

			_unref(m_ptr);
			m_ptr = _data;

			rc = 1;
		}
		return rc; // return 1.
	}


	template <class T>
	CowData<T>::~CowData()
	{
		_unref((void*)m_ptr);
	}


	template <class T>
	template <bool p_ensure_zero>
	Error CowData<T>::resize(int p_size) {
		ERR_FAIL_COND_V(p_size < 0, ERR_INVALID_PARAMETER);
		int current_size = size();

		if (p_size == current_size) {
			return OK;
		}

		if (p_size == 0) {
			// wants to clean up
			_unref(m_ptr);
			m_ptr = nullptr;
			return OK;
		}

		// possibly changing size, copy on write
		uint32_t rc = _copy_on_write();

		size_t current_alloc_size = _get_alloc_size(current_size);
		size_t alloc_size = _get_alloc_size(p_size);
		// L_PRINT("resize to", p_size, current_alloc_size,alloc_size);

		// ERR_FAIL_COND_V(!_get_alloc_size_checked(p_size, &alloc_size), ERR_OUT_OF_MEMORY);

		if (p_size > current_size) {
			if (alloc_size != current_alloc_size) {
				if (current_size == 0) {
					// alloc from scratch
					uint32_t* ptr = (uint32_t*)Memory::alloc_static(alloc_size, true);
					ERR_FAIL_COND_V(!ptr, ERR_OUT_OF_MEMORY);
					*(ptr - 1) = 0; //size, currently none
					new (ptr - 2) s_ui32(1); //refcount
					m_ptr = (T*)ptr;

				}
				else {
					uint32_t* _ptrnew = (uint32_t*)Memory::realloc_static(m_ptr, alloc_size, true);
					ERR_FAIL_COND_V(!_ptrnew, ERR_OUT_OF_MEMORY);
					new (_ptrnew - 2) s_ui32(rc); //refcount

					m_ptr = (T*)(_ptrnew);
				}
			}

			// construct the newly created elements

			if (!std::is_trivially_constructible<T>::value) {
				for (int i = *_get_size(); i < p_size; i++) {
					memnew_placement(&m_ptr[i], T);
				}
			}
			else if (p_ensure_zero) {
				memset((void*)(m_ptr + current_size), 0, (p_size - current_size) * sizeof(T));
			}

			*_get_size() = p_size;

		}

		else if (p_size < current_size) {
			if (!std::is_trivially_destructible<T>::value) {
				// deinitialize no longer needed elements
				for (uint32_t i = p_size; i < *_get_size(); i++) {
					T* t = &m_ptr[i];
					t->~T();
				}
			}

			if (alloc_size != current_alloc_size) {
				uint32_t* _ptrnew = (uint32_t*)Memory::realloc_static(m_ptr, alloc_size, true);
				ERR_FAIL_COND_V(!_ptrnew, ERR_OUT_OF_MEMORY);
				// new with placement 
				new (_ptrnew - 2) s_ui32(rc); //refcount

				m_ptr = (T*)(_ptrnew);
			}

			*_get_size() = p_size;
		}

		return OK;
	}

	template <class T>
	int CowData<T>::Find(const T& p_val, int p_from) const {
		int ret = -1;

		// highest bit
		if (p_from < 0 || size() == 0) {
			return ret;
		}

		for (ui32 i = static_cast<ui32>(p_from); i < size(); i++) {
			if (get(i) == p_val) {
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
	int CowData<T>::count(const T& p_val) const {
		int amount = 0;
		for (int i = 0; i < size(); i++) {
			if (get(i) == p_val)
				amount++;
		}
		return amount;
	}
}
#endif // !__COWDATA_H__
