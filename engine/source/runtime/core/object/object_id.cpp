#pragma once
#include "object_id.h"

namespace lain {

	s_u32 ObjectIDAllocator::m_next_id = s_u32(0);
	u32 ObjectIDAllocator::Alloc() {
		u32 id =  m_next_id.conditional_increment();
		if (id >= k_invalid_gobject_id) {
			L_ERROR(__FUNCTION__, "gobject id overflow");
		}
		return id;
	}
}
