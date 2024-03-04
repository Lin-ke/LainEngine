#pragma once
#ifndef __CORE_OBJECT_H__
#define __CORE_OBJECT_H__
#include "core/object/object_id.h"
#include "signal.h"
#include "core/os/spin_lock.h"
#include "core/templates/list.h"
#include "core/variant/callable.h"
#include "core/templates/hash_map.h"
#include "core/math/hashfuncs.h"
#include "core/meta/reflection/reflection.h"
// base class of all object
namespace lain {
	REFLECTION_TYPE(Object)
		CLASS(Object, WhiteListFields)
	{
		REFLECTION_BODY(Object);
	public:
		Object() {}
		Object(ObjectID id) { m_instance_id = id; m_type_is_reference = false; }
		Object::Object(bool p_reference) {
			_construct_object(p_reference);
		}
	struct Connection {
		Signal signal;
		Callable callable;

		uint32_t flags = 0;
		bool operator<(const Connection& p_conn) const;

		operator Variant() const;

		Connection() {}
		Connection(const Variant& p_variant);

	};
	L_INLINE bool is_ref_counted() const { return m_type_is_reference; }
	ObjectID get_instance_id() const { return m_instance_id; }
private:
	ObjectID m_instance_id;
	bool m_type_is_reference = false;

	void _construct_object(bool p_reference);
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
	
	};

}
// const 限定符是必要的，因为const对象拒绝调用非const方法

#endif // !__CORE_OBJECT_H__
