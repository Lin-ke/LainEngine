#include "objectdb.h"

namespace lain{
	SpinLock ObjectDB::spin_lock;
	uint32_t ObjectDB::slot_count = 0;
	uint32_t ObjectDB::slot_max = 0;
	ObjectDB::ObjectSlot* ObjectDB::object_slots = nullptr;
	uint64_t ObjectDB::validator_counter = 0;
	ObjectID ObjectDB::add_instance(Object* p_object) {
		spin_lock.lock();

		if (unlikely(slot_count == slot_max)) {
			CRASH_COND(slot_count == (1 << OBJECTDB_SLOT_MAX_COUNT_BITS));

			uint32_t new_slot_max = slot_max > 0 ? slot_max * 2 : 1;
			object_slots = (ObjectSlot*)memrealloc(object_slots, sizeof(ObjectSlot) * new_slot_max);
			for (uint32_t i = slot_max; i < new_slot_max; i++) {
				object_slots[i].object = nullptr;
				object_slots[i].is_ref_counted = false;
				object_slots[i].next_free = i;
				object_slots[i].validator = 0;
			}
			slot_max = new_slot_max;
		}
		uint32_t slot = object_slots[slot_count].next_free;
		if (object_slots[slot].object != nullptr) {
			//spin_lock.unlock();
			ERR_FAIL_COND_V(object_slots[slot].object != nullptr, ObjectID());
			spin_lock.unlock();

		}
		object_slots[slot].object = p_object;
		object_slots[slot].is_ref_counted = p_object->is_ref_counted();
		validator_counter = (validator_counter + 1) & OBJECTDB_VALIDATOR_MASK;
		if (unlikely(validator_counter == 0)) {
			validator_counter = 1;
		}
		object_slots[slot].validator = validator_counter;

		uint64_t id = validator_counter;
		id <<= OBJECTDB_SLOT_MAX_COUNT_BITS;
		id |= uint64_t(slot);

		if (p_object->is_ref_counted()) {
			id |= OBJECTDB_REFERENCE_BIT;
		}

		slot_count++;

		spin_lock.unlock();

		return ObjectID(id);
	}

	void ObjectDB::remove_instance(Object* p_object) {
		spin_lock.lock();

		uint64_t t = p_object->get_instance_id();
		uint32_t slot = t & OBJECTDB_SLOT_MAX_COUNT_MASK; //slot is always valid on valid object
		//decrease slot count
		slot_count--;
		//set the free slot properly
		object_slots[slot_count].next_free = slot;
		//invalidate, so checks against it fail
		object_slots[slot].validator = 0;
		object_slots[slot].is_ref_counted = false;
		object_slots[slot].object = nullptr;
		spin_lock.unlock();

	}


	Object* ObjectDB::get_instance(ObjectID p_instance_id) {
		uint64_t id = p_instance_id;
		uint32_t slot = id & OBJECTDB_SLOT_MAX_COUNT_MASK;

		ERR_FAIL_COND_V(slot >= slot_max, nullptr); // This should never happen unless RID is corrupted.

		spin_lock.lock();

		uint64_t validator = (id >> OBJECTDB_SLOT_MAX_COUNT_BITS) & OBJECTDB_VALIDATOR_MASK;

		if (unlikely(object_slots[slot].validator != validator)) {
			spin_lock.unlock();
			return nullptr;
		}

		Object* object = object_slots[slot].object;

		spin_lock.unlock();

		return object;
	}
}