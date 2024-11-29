#include "refcounted.h"
#include "core/object/objectdb.h"
namespace lain {

bool RefCounted::init_ref() {
	if (reference()) {
		if (!is_referenced() && refcount_init.unref()) {
			unreference();  // first referencing is already 1, so compensate for the ref above
		}
		return true;
	}
	return false;

}	
bool RefCounted::reference() {
	uint32_t rc_val = refcount.refval();
	bool success = rc_val != 0;
	return success;
}
bool RefCounted::unreference() {
	uint32_t rc_val = refcount.unrefval();
	bool die = rc_val == 0;
	return die;

}
int RefCounted::get_reference_count() const {
	return refcount.get();

}

RefCounted::RefCounted() :Object(true){
	refcount.init();
	refcount_init.init();
}

void WeakRef::_bind_methods() {}

Variant WeakRef::get_ref() const {
  if (ref.is_null()) {
    return Variant();
  }

  Object* obj = ObjectDB::get_instance(ref);
  if (!obj) {
    return Variant();
  }
  RefCounted* r = cast_to<RefCounted>(obj);
  if (r) {
    return Ref<RefCounted>(r);
  }

  return obj;
}

void WeakRef::set_obj(Object *p_object) {
	ref = p_object ? p_object->get_instance_id() : ObjectID();
}

void WeakRef::set_ref(const Ref<RefCounted> &p_ref) {
	ref = p_ref.is_valid() ? p_ref->get_instance_id() : ObjectID();
}

// void WeakRef::_bind_methods() {
// 	ClassDB::bind_method(D_METHOD("get_ref"), &WeakRef::get_ref);
// }

}
