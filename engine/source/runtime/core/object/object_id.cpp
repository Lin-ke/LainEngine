#include "object_id.h"

s_u32 lain::ObjectIDAllocator::m_next_id(0);
u32 lain::ObjectIDAllocator::Alloc() {
	u32 id =  m_next_id.conditional_increment();
	if (id >= k_invalid_gobject_id) {
		L_ERROR(__FUNCTION__, "gobject id overflow");
	}
	return id;
}