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
#include "core/object/object_id.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"

// Macros
#define LCLASS(m_class, m_inherits)		\
private:								\
	void operator=(const m_class &p_rval) {} \
									\
public:\
		\
	virtual String get_class()	const override 	\
		{return String(#m_class);}	\
									\
	virtual char* get_c_class() const override \
	{return #m_class;} \
	virtual StringName *_get_class_namev() const override{\
		static StringName _class_name_static;\
		if (unlikely(!_class_name_static)){ \
			StringName::assign_static_unique_class_name(&_class_name_static, #m_class); 		\
		} return &_class_name_static; \
	}	\
		\
	static _FORCE_INLINE_ String get_class_static() {                     \
		return String(#m_class);										   \
	}																	\
				\
	static _FORCE_INLINE_ String get_parent_class_static() {						\
			return String(m_inherits::get_class_static());					   \
	}									\
										\
private:							


// base class of all object
namespace lain {
	class Viewport;
	// signal mechanism
	REFLECTION_TYPE(Connection)
		CLASS(Connection, WhiteListFields) {
		REFLECTION_BODY(Connection);
public:
	META(Enable)
		Signal signal;
	Callable callable; // 什么类型的什么方法


	uint32_t flags = 0;
	bool operator<(const Connection & p_conn) const;

	operator Variant() const;

	Connection() {}
	Connection(const Variant & p_variant);

	};

	/// object


	REFLECTION_TYPE(Object)
		CLASS(Object, WhiteListFields)
	{
		REFLECTION_BODY(Object);
	public:
		Object() { _construct_object(false); }
		Object(ObjectID id) { m_instance_id = id; m_type_is_reference = false; }
		Object::Object(bool p_reference) {
			_construct_object(p_reference);
		}
		// META
		HashMap<StringName, Variant> metadata;
		HashMap<StringName, Variant*> metadata_properties;
		mutable const StringName* _class_name_ptr = nullptr;

	L_INLINE bool is_ref_counted() const { return m_type_is_reference; }
	ObjectID get_instance_id() const { return m_instance_id; }
	/// static method
	template <class T>
	static T* cast_to(Object* p_object) {
		return dynamic_cast<T*>(p_object);
	}

	template <class T>
	static const T* cast_to(const Object* p_object) {
		return dynamic_cast<const T*>(p_object);
	}
	virtual String get_class() const { return String("Object"); }
	virtual char* get_c_class() const { return "Object"; }
	static String get_class_static() { return String("Object"); }
	
	virtual const StringName* _get_class_namev() const {
		static StringName _class_name_static;
		if (unlikely(!_class_name_static)) {
			StringName::assign_static_unique_class_name(&_class_name_static, "Object");
		}
		return &_class_name_static;
	}
	_FORCE_INLINE_ const StringName& get_class_name() const {
		//if (_extension) {
		//	// Can't put inside the unlikely as constructor can run it
		//	return _extension->class_name;
		//}

		if (unlikely(!_class_name_ptr)) {
			// While class is initializing / deinitializing, constructors and destructurs
			// need access to the proper class at the proper stage.
			return *_get_class_namev();
		}
		return *_class_name_ptr;
	}

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
