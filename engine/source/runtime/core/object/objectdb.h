#pragma once
#ifndef __OBJECT_DB_H__
#define __OBJECT_DB_H__
#define OBJECTDB_VALIDATOR_BITS 39
#define OBJECTDB_VALIDATOR_MASK ((uint64_t(1) << OBJECTDB_VALIDATOR_BITS) - 1)
#define OBJECTDB_SLOT_MAX_COUNT_BITS 24
#define OBJECTDB_SLOT_MAX_COUNT_MASK ((uint64_t(1) << OBJECTDB_SLOT_MAX_COUNT_BITS) - 1)
#define OBJECTDB_REFERENCE_BIT (uint64_t(1) << (OBJECTDB_SLOT_MAX_COUNT_BITS + OBJECTDB_VALIDATOR_BITS))

#include "base.h"
#include "object.h"
#include "core/os/spin_lock.h"
namespace lain {
	class ObjectDB {
	public:
		struct ObjectSlot { // 128 bits per slot.
			uint64_t validator : OBJECTDB_VALIDATOR_BITS;
			uint64_t next_free : OBJECTDB_SLOT_MAX_COUNT_BITS;
			uint64_t is_ref_counted : 1;
			Object* object = nullptr;
		};
		static uint32_t slot_count;
		static uint32_t slot_max;
		static Object p;
		static ObjectSlot* object_slots;
		static uint64_t validator_counter;
		static SpinLock spin_lock;
		static void cleanup();
		// INLINE
		L_INLINE static Object* get_instance(ObjectID p_instance_id);

		static ObjectID add_instance(Object* p_object);
		static void remove_instance(Object* p_object);
		static void setup() {}

	};
}
#endif // !__OBJECT_DB_H__
