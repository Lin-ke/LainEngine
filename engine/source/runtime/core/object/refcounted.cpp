#include "refcounted.h"
namespace lain {

bool RefCounted::init_ref() {
	if (reference()) {
		if (!is_referenced() && refcount_init.unref()) {
			unreference();
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
}
}
