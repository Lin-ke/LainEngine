#ifndef __OBJECT_ID_H__
#define __OBJECT_ID_H__
#include "base.h"
#include "core/templates/safe_numeric.h"
namespace lain {
	using GObjectID = uint32_t;
	constexpr GObjectID k_invalid_gobject_id = std::numeric_limits<std::uint32_t>::max();
	class ObjectIDAllocator {
	public:
		L_INLINE static GObjectID Alloc();
	private:
		static s_u32 m_next_id;
	};
}

#endif // !__OBJECT_ID_H__
