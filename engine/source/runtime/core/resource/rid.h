#pragma once
#ifndef RID_H
#define RID_H

#include "base.h"
#include "core/templates/safe_numeric.h"
namespace lain {
	// thread safety allocator
class RIDAllocator {
public:
	L_INLINE static u64 Alloc() {
		return m_next_id.conditional_increment();
	}
private:
	static s_u64 m_next_id;
};
//A handle for a Resource's unique identifier.
//Description
//
//The RID Variant type is used to access a low - level resource by its unique ID.RIDs are opaque, which means they do not grant access to the resource by themselves.They are used by the low - level server classes, such as DisplayServer, RenderingServer, TextServer, etc.
class RID {
	uint64_t _id = 0;

public:
	_ALWAYS_INLINE_ bool operator==(const RID& p_rid) const {
		return _id == p_rid._id;
	}
	_ALWAYS_INLINE_ bool operator<(const RID& p_rid) const {
		return _id < p_rid._id;
	}
	_ALWAYS_INLINE_ bool operator<=(const RID& p_rid) const {
		return _id <= p_rid._id;
	}
	_ALWAYS_INLINE_ bool operator>(const RID& p_rid) const {
		return _id > p_rid._id;
	}
	_ALWAYS_INLINE_ bool operator>=(const RID& p_rid) const {
		return _id >= p_rid._id;
	}
	_ALWAYS_INLINE_ bool operator!=(const RID& p_rid) const {
		return _id != p_rid._id;
	}
	_ALWAYS_INLINE_ bool is_valid() const { return _id != 0; }
	_ALWAYS_INLINE_ bool is_null() const { return _id == 0; }

	_ALWAYS_INLINE_ uint32_t get_local_index() const { return _id & 0xFFFFFFFF; }

	static _ALWAYS_INLINE_ RID from_uint64(uint64_t p_id) {
		RID _rid;
		_rid._id = p_id;
		return _rid;
	}
	_ALWAYS_INLINE_ uint64_t get_id() const { return _id; }

	_ALWAYS_INLINE_ RID() {}
};
}

#endif // RID_H
