#ifndef CALLABLE_METHOD_POINTER_H
#define CALLABLE_METHOD_POINTER_H
#include "core/object/object.h"
#include "core/variant/callable.h"

namespace lain{
class CallableCustomMethodPointerBase : public CallableCustom {
	virtual uint32_t hash() const;

};
template <typename T, typename ... P>
class CallableCustomMethodPointer : public CallableCustomMethodPointerBase {
  T* instance;
  void (T::*method)(P...);
  uint64_t object_id;
  public:
  virtual ObjectID get_object() const {
		if (ObjectDB::get_instance(ObjectID(data.object_id)) == nullptr) {
			return ObjectID();
		}
		return data.instance->get_instance_id();
	}
  
	virtual int get_argument_count(bool &r_is_valid) const {
		r_is_valid = true;
		return sizeof...(P);
	}

  virtual void call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, Callable::CallError &r_call_error) const {
		ERR_FAIL_NULL_MSG(ObjectDB::get_instance(ObjectID(data.object_id)), "Invalid Object id '" + uitos(data.object_id) + "', can't call method.");
		call_with_variant_args(data.instance, data.method, p_arguments, p_argcount, r_call_error);
	}

};
}
#endif