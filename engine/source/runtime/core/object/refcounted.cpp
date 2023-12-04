#include "refcounted.h"
namespace lain {

bool RefCounted::init_ref() {
	return false;

}	
bool RefCounted::reference() {
	refcount.refval();
	return false;
}
bool RefCounted::unreference() {
	return false;

}
int RefCounted::get_reference_count() const {
	return 0;

}

RefCounted::RefCounted() {

}
}
