#pragma once
#ifndef __CORE_OBJECT_H__
#define __CORE_OBJECT_H__
#include "core/object/object_id.h"
#include "signal.h"
#include "core/templates/list.h"
#include "core/variant/callable.h"
#include "core/templates/hash_map.h"
#include "core/math/hashfuncs.h"
// base class of all object
namespace lain {
	REFLECTION_TYPE(Object)
		CLASS(Object, WhiteListFields){
public:

	Object(ObjectID id) { m_instance_id = id; }
	Object() {}
	struct Connection {
		Signal signal;
		Callable callable;

		uint32_t flags = 0;
		bool operator<(const Connection& p_conn) const;

		operator Variant() const;

		Connection() {}
		Connection(const Variant& p_variant);
	};
private:
	ObjectID m_instance_id;
	struct SignalData {
		struct Slot {
			int reference_count = 0;
			Connection conn;
			List<Connection>::Element* cE = nullptr;
		};
		//MethodInfo user;
		 HashMap<Callable, Slot, HashableHasher<Callable>> slot_map;
	};
	 HashMap<StringName, SignalData> signal_map;

	// for gc
	
	// signal (event)
};
}


#endif // !__CORE_OBJECT_H__
