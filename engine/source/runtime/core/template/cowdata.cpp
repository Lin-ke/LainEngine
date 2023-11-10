#include "cowdata.h"

template <typename T>
void CowData<T>::_Ref(const CowData<T> p_from){
	if (m_ptr == p_from.m_ptr) {
		return;
	}


}

template <typename T>

void CowData<T>::_Unref(const CowData<T> p_from) {

}
