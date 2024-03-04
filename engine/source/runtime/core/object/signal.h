#pragma once
#include "base.h"
#include "object_id.h"
#include "core/string/string_name.h"
namespace lain{
	class Object;
	class Variant;
class Signal {
	alignas(8) StringName name;
	ObjectID object;
	
	public:
		_FORCE_INLINE_ bool is_null() const {
			return object.is_null() && name == StringName();
		}
		Object* get_object() const;
		_FORCE_INLINE_ ObjectID get_object_id() const { return object; }
		_FORCE_INLINE_ StringName get_name() const { return name; }

		bool operator==(const Signal& p_signal) const;
		bool operator!=(const Signal& p_signal) const;
		bool operator<(const Signal& p_signal) const;

		operator String() const;

		Error emit(const Variant** p_arguments, int p_argcount) const;
		Error connect(const Callable& p_callable, uint32_t p_flags = 0);
		void disconnect(const Callable& p_callable);
		bool is_connected(const Callable& p_callable) const;

		Array get_connections() const;
		Signal(const Object* p_object, const StringName& p_name);
		Signal(ObjectID p_object, const StringName& p_name);
		Signal() {}
	};


}
